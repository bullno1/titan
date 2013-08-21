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

#include "engine.h"
#include "Titan.h"

#include <SDL.h>
#include <SDL_opengl.h>

using namespace std;

namespace InputDevice
{
	enum Enum
	{
		Main,

		/*
		Joystick1,
		Joystick2,
		Joystick3,
		Joystick4,
		*/

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

namespace {
	bool windowOpened = false;
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

class AKUContext
{
public:
	AKUContext()
		:mContextId(AKUCreateContext())
	{}

	~AKUContext()
	{
		AKUDeleteContext(mContextId);
		AKUFinalize();
	}

	AKUContextID mContextId;
};

void openWindow(const char* title, int width, int height)
{
	if((SDL_WasInit(0) & (SDL_INIT_VIDEO)) == 0)
	{
		SDL_Init(SDL_INIT_VIDEO);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	}

	SDL_SetVideoMode(width, height, 32, SDL_OPENGL);
	SDL_WM_SetCaption(title, 0);
	AKUDetectGfxContext();
	AKUSetScreenSize(width, height);

	windowOpened = true;
}

void closeWindow()
{
	AKUReleaseGfxContext();

	windowOpened = false;
}

void handleKeyboardEvent(SDL_KeyboardEvent event, bool down)
{
	AKUEnqueueKeyboardEvent(InputDevice::Main, MainSensor::RawKeyboard, event.keysym.sym, down);
	AKUEnqueueKeyboardEvent(InputDevice::Main, MainSensor::TextInput, event.keysym.unicode, down);
}

void handleMouseButtonEvent(Uint8 button, bool down)
{
	switch(button)
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
	case SDL_BUTTON_WHEELDOWN:
		AKUEnqueueWheelEvent(InputDevice::Main, MainSensor::MouseWheel, -1.0f);
		break;
	case SDL_BUTTON_WHEELUP:
		AKUEnqueueWheelEvent(InputDevice::Main, MainSensor::MouseWheel,  1.0f);
		break;
	}
}

void injectInput(SDL_Event event)
{
	switch(event.type)
	{
	case SDL_KEYDOWN:
		handleKeyboardEvent(event.key, true);
		break;
	case SDL_KEYUP:
		handleKeyboardEvent(event.key, false);
		break;
	case SDL_MOUSEBUTTONDOWN:
		handleMouseButtonEvent(event.button.button, true);
		break;
	case SDL_MOUSEBUTTONUP:
		handleMouseButtonEvent(event.button.button, false);
		break;
	case SDL_MOUSEMOTION:
		AKUEnqueuePointerEvent(InputDevice::Main, MainSensor::Mouse, event.motion.x, event.motion.y);
		break;
	}
}

void InitSDL()
{
	SDL_Init(SDL_INIT_VIDEO);
}

ExitReason::Enum engineMain(int argc, char* argv[])
{
	cout << "-------------------------------" << endl
	     << "Initializing Titan" << endl
	     << "-------------------------------" << endl;

	Init sdl(InitSDL, SDL_Quit);

	//Initialize AKU
	AKUContext context;
	AKUSetArgv(argv);

	//Initialize AKU subsystems
	Init util(AKUInitializeUtil, AKUFinalizeUtil);
	Init sim(AKUInitializeSim, AKUFinalizeSim);
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
	AKURunBytecode(moai_lua, moai_lua_SIZE);

	//Register host addons
	REGISTER_LUA_CLASS(Titan);
	AKUSetFunc_OpenWindow(openWindow);

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

	if(!windowOpened)
	{
	    cout << "-------------------------------" << endl
		     << "Failed to create render window" << endl
			 << "-------------------------------" << endl;
		return ExitReason::Error;
	}

	//Main loop
	bool running = true;
	Titan& titan = Titan::Get();
	while(titan.Update() && running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				closeWindow();
				return ExitReason::UserAction;
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_r && (SDL_GetModState() & KMOD_CTRL))
				{
					closeWindow();
					return ExitReason::Restart;
				}
				break;
			}

			injectInput(event);
		}

		AKUUpdate();
		glClear(GL_COLOR_BUFFER_BIT);
		AKURender();
		SDL_GL_SwapBuffers();
	}

	return ExitReason::Restart;
}