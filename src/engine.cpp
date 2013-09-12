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

#include <GLFW/glfw3.h>

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
	GLFWwindow* window = NULL;
	ExitReason::Enum exitReason;
	bool running;
	int windowX = -1;
	int windowY = -1;
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

class AKUContextWrapper
{
public:
	AKUContextWrapper()
		:mContextId(AKUCreateContext())
	{}

	~AKUContextWrapper()
	{
		AKUDeleteContext(mContextId);
		AKUFinalize();
	}

	AKUContextID mContextId;
};

void handleMouseMove(GLFWwindow* window, double x, double y)
{
	AKUEnqueuePointerEvent(InputDevice::Main, MainSensor::Mouse, (int)x, (int)y);
}

void handleMouseButton(GLFWwindow* window, int button, int action, int mod)
{
	switch(button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		AKUEnqueueButtonEvent(InputDevice::Main, MainSensor::MouseLeft, action == GLFW_PRESS);
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		AKUEnqueueButtonEvent(InputDevice::Main, MainSensor::MouseMiddle, action == GLFW_PRESS);
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		AKUEnqueueButtonEvent(InputDevice::Main, MainSensor::MouseRight, action == GLFW_PRESS);
		break;
	}
}

void handleTextInput(GLFWwindow* window, unsigned int character)
{
	AKUEnqueueKeyboardEvent(InputDevice::Main, MainSensor::TextInput, character, true);
	AKUEnqueueKeyboardEvent(InputDevice::Main, MainSensor::TextInput, character, false);
}

void handleScroll(GLFWwindow* window, double x, double y)
{
	AKUEnqueueWheelEvent(InputDevice::Main, MainSensor::RawKeyboard, (float)y);
}

void closeWindow()
{
	AKUReleaseGfxContext();
	glfwGetWindowPos(window, &windowX, &windowY);
	glfwDestroyWindow(window);
	window = NULL;
	running = false;
}

void handleKey(GLFWwindow* window, int keycode, int scancode, int action, int modifier)
{
	if((action == GLFW_PRESS) && (keycode == GLFW_KEY_R) && ((modifier & GLFW_MOD_CONTROL) > 0))
	{
		closeWindow();
		exitReason = ExitReason::Restart;
	}
	else
	{
		if(action != GLFW_REPEAT)
			AKUEnqueueKeyboardEvent(InputDevice::Main, MainSensor::RawKeyboard, keycode, action == GLFW_PRESS);
	}
}

void handleClose(GLFWwindow* window)
{
	closeWindow();
	exitReason = ExitReason::UserAction;
}

void openWindow(const char* title, int width, int height)
{
	if(window) return;

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(width, height, title, NULL, NULL);
	if(windowX > 0 && windowY > 0)
	{
		glfwSetWindowPos(window, windowX, windowY);
	}

	glfwSwapInterval(1);
	glfwSetCursorPosCallback(window, handleMouseMove);
	glfwSetCharCallback(window, handleTextInput);
	glfwSetMouseButtonCallback(window, handleMouseButton);
	glfwSetScrollCallback(window, handleScroll);
	glfwSetKeyCallback(window, handleKey);
	glfwSetWindowCloseCallback(window, handleClose);

	glfwMakeContextCurrent(window);
	AKUDetectGfxContext();
	AKUSetScreenSize(width, height);
}

void InitGLFW()
{
	glfwInit();
}

ExitReason::Enum engineMain(int argc, char* argv[])
{
	cout << "-------------------------------" << endl
	     << "Initializing Titan" << endl
	     << "-------------------------------" << endl;

	Init glfw(InitGLFW, glfwTerminate);

	//Initialize AKU
	AKUContextWrapper context;
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
	AKURunData(moai_lua, moai_lua_SIZE, AKU_DATA_STRING, AKU_DATA_ZIPPED);

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

	if(!window)
	{
	    cout << "-------------------------------" << endl
		     << "Failed to create render window" << endl
			 << "-------------------------------" << endl;
		return ExitReason::Error;
	}

	//Main loop
	running = true;
	Titan& titan = Titan::Get();
	exitReason = ExitReason::Restart;
	while(titan.Update() && running)
	{
		AKUUpdate();
		glClear(GL_COLOR_BUFFER_BIT);
		AKURender();
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	return exitReason;
}