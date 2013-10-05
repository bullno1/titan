#include <moai-core/host.h>
#include <moai-sim/host.h>
#include <moai-chipmunk/host.h>
#include <moai-box2d/host.h>
#include <moai-http-client/host.h>
#include <moai-http-server/host.h>
#include <moai-luaext/host.h>
#include <moai-untz/host.h>
#include <moai-util/host.h>
#include <moai-audiosampler/AKU-audiosampler.h>
#include <lua-headers/moai_lua.h>
#include <iostream>

#include "sim.h"
#include "Titan.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_keycode.h>

using namespace std;

namespace InputDevice
{
	enum Enum
	{
		Main,

		Count
	};
}

namespace MainSensor
{
	enum Enum
	{
		RawKeyboard,
		TextInput,

		Mouse,
		MouseWheel,
		MouseLeft,
		MouseMiddle,
		MouseRight,

		Count
	};
}

class Init
{
public:
	typedef void(*InitFunc)();
	typedef void(*ShutdownFunc)();

	Init(InitFunc initFunc, ShutdownFunc shutdownFunc)
		:mShutdownFunc(shutdownFunc)
	{
		initFunc();
	}

	~Init()
	{
		mShutdownFunc();
	}

private:
	ShutdownFunc mShutdownFunc;
};

template<typename T>
class AKUContextWrapper
{
public:
	AKUContextWrapper(T* userData)
	{
		mContextId = AKUCreateContext();
		AKUSetUserdata(userData);
	}

	~AKUContextWrapper()
	{
		AKUDeleteContext(mContextId);
		AKUFinalize();
	}

private:
	AKUContextID mContextId;
};

class Sim
{
public:
	Sim(int argc, char* argv[])
		:mArgc(argc)
		,mArgv(argv)
		,mWindow(NULL)
		,mGLContext(NULL)
		,mWindowX(SDL_WINDOWPOS_UNDEFINED)
		,mWindowY(SDL_WINDOWPOS_UNDEFINED)
	{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	}

	~Sim()
	{
		SDL_Quit();
		AKUFinalize();
	}

	ExitReason::Enum run()
	{
		cout << "-------------------------------" << endl
			 << "Initializing Titan" << endl
			 << "-------------------------------" << endl;

		//Initialize AKU
		AKUContextWrapper<Sim> context(this);
		AKUSetArgv(mArgv);

		//Initialize AKU subsystems
		Init sim(AKUInitializeSim, AKUFinalizeSim);
		Init util(AKUInitializeUtil, AKUFinalizeUtil);
		Init chipmunk(AKUInitializeChipmunk, AKUFinalizeChipmunk);
		Init box2d(AKUInitializeBox2D, AKUFinalizeBox2D);
		Init httpServer(AKUInitializeHttpServer, AKUFinalizeHttpServer);
		Init httpClient(AKUInitializeHttpClient, AKUFinalizeHttpClient);
		AKUInitializeUntz();

		//Initialize Lua libraries
		AKUExtLoadLuacrypto();
		AKUExtLoadLuacurl();
		AKUExtLoadLuafilesystem();
		AKUExtLoadLuasocket();
		AKUExtLoadLuasql();
		AKURunData(moai_lua, moai_lua_SIZE, AKU_DATA_STRING, AKU_DATA_ZIPPED);

		//Register host addons
		REGISTER_LUA_CLASS(Titan);
		AKUSetFunc_OpenWindow(thunk_openWindow);

		//Initialize input
		AKUSetInputConfigurationName("AKUTitan");
		AKUReserveInputDevices(InputDevice::Count);

		//Main input device
		AKUSetInputDevice(InputDevice::Main, "device");
		AKUReserveInputDeviceSensors(InputDevice::Main, MainSensor::Count);
		AKUSetInputDeviceKeyboard(InputDevice::Main, MainSensor::RawKeyboard, "keyboard");
		AKUSetInputDeviceKeyboard(InputDevice::Main, MainSensor::TextInput, "textInput");
		AKUSetInputDevicePointer(InputDevice::Main, MainSensor::Mouse, "mouse");
		AKUSetInputDeviceWheel(InputDevice::Main, MainSensor::MouseWheel, "mouseWheel");
		AKUSetInputDeviceButton(InputDevice::Main, MainSensor::MouseLeft, "mouseLeft");
		AKUSetInputDeviceButton(InputDevice::Main, MainSensor::MouseMiddle, "mouseMiddle");
		AKUSetInputDeviceButton(InputDevice::Main, MainSensor::MouseRight, "mouseRight");

		//Run main script
		AKURunScript("main.lua");

		if(!mWindow)
		{
			cout << "-------------------------------" << endl
				 << "Failed to create render window" << endl
				 << "-------------------------------" << endl;
			return ExitReason::Error;
		}

		//Main loop
		mRunning = true;
		Titan& titan = Titan::Get();
		mExitReason = ExitReason::Restart;
		while(titan.Update() && mRunning)
		{
			SDL_Event event;
			while(SDL_PollEvent(&event))
				processEvent(event);

			AKUUpdate();
			glClear(GL_COLOR_BUFFER_BIT);
			AKURender();
			SDL_GL_SwapWindow(mWindow);
		}

		return mExitReason;
	}

private:
	void openWindow(const char* title, int width, int height)
	{
		if(mWindow) return;

		mWindow = SDL_CreateWindow(title, mWindowX, mWindowY, width, height, SDL_WINDOW_OPENGL);
		if(SDL_GL_SetSwapInterval(-1) < 0)
			SDL_GL_SetSwapInterval(1);
		mGLContext = SDL_GL_CreateContext(mWindow);

		AKUDetectGfxContext();
		AKUSetScreenSize(width, height);
	}

	void closeWindow()
	{
		if(!mWindow) return;

		AKUReleaseGfxContext();

		SDL_GL_DeleteContext(mGLContext);
		SDL_GetWindowPosition(mWindow, &mWindowX, &mWindowY);
		SDL_DestroyWindow(mWindow);
		mWindow = NULL;
		mGLContext = NULL;
		mRunning = false;
	}

	void processEvent(SDL_Event event)
	{
		switch(event.type)
		{
		case SDL_MOUSEMOTION:
			AKUEnqueuePointerEvent(InputDevice::Main, MainSensor::Mouse, (int)event.motion.x, (int)event.motion.y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			handleMouseButton(event.button);
			break;
		case SDL_MOUSEWHEEL:
			AKUEnqueueWheelEvent(InputDevice::Main, MainSensor::RawKeyboard, (float)event.wheel.y);
			break;
		case SDL_KEYDOWN:
			handleKey(event.key, true);
			break;
		case SDL_KEYUP:
			handleKey(event.key, false);
			break;
		case SDL_QUIT:
			handleQuit();
			break;
		}
	}

	void handleMouseButton(SDL_MouseButtonEvent event)
	{
		bool down = event.state == SDL_PRESSED;
		switch(event.button)
		{
		case SDL_BUTTON_LEFT:
			AKUEnqueueButtonEvent(InputDevice::Main, MainSensor::MouseLeft, down);
			break;
		case SDL_BUTTON_MIDDLE:
			AKUEnqueueButtonEvent(InputDevice::Main, MainSensor::MouseMiddle, down);
			break;
		case SDL_BUTTON_RIGHT:
			AKUEnqueueButtonEvent(InputDevice::Main, MainSensor::MouseRight, down);
			break;
		}
	}

	void handleKey(SDL_KeyboardEvent event, bool down)
	{
		if(event.repeat) return;

		if(down && (event.keysym.sym == SDLK_r) && ((event.keysym.mod & KMOD_CTRL) > 0))
		{
			closeWindow();
			mExitReason = ExitReason::Restart;
		}
		else
		{
			AKUEnqueueKeyboardEvent(InputDevice::Main, MainSensor::RawKeyboard, event.keysym.sym, down);
		}
	}

	void handleQuit()
	{
		closeWindow();
		mExitReason = ExitReason::UserAction;
	}

	static void thunk_openWindow(const char* title, int width, int height)
	{
		static_cast<Sim*>(AKUGetUserdata())->openWindow(title, width, height);
	}

	int mArgc;
	char** mArgv;
	SDL_Window* mWindow;
	SDL_GLContext mGLContext;
	ExitReason::Enum mExitReason;
	bool mRunning;
	int mWindowX;
	int mWindowY;
};

Sim* createSim(int argc, char* argv[])
{
	return new Sim(argc, argv);
}

ExitReason::Enum runSim(Sim* sim)
{
	return sim->run();
}

void destroySim(Sim* sim)
{
	delete sim;
}