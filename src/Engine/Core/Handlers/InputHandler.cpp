#include "InputHandler.h"

#include <../../Types/Input/KeyEvents.h>
#include <Engine/Util/Log.h>
#include <Engine/Util/BitwiseMacros.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/EngineDefines.h>

#include <Engine/imgui/imgui.h>
#include <Engine/imgui/backends/imgui_impl_sdl3.h>
#include <Engine/Core/System/Memory.h>
using namespace WEngine;

float Sint16ToFloat(const int16 num)
{
	float normalized;
	if (num < 0)
		normalized = (float)num / 32768.0f;
	else
		normalized = (float)num / 32767.0f;
	return normalized;
}

// hardest thing is programming is naming things
// this take a -1 to 1 float and makes it 0 to 1
float FloatRange1_1To0_1(const float num)
{
	return (num + 1.0f) / 2.0f;
}

InputHandler::InputHandler()
{
	int numkeys;
	SDL_GetKeyboardState(&numkeys);
	m_keystate = (bool*)wNew(numkeys);
	m_lastKeystate = (bool*)wNew(numkeys);

	// prevents random key presses on boot

	for (int i = 0; i < numkeys; i++)
	{
		m_keystate[i] = false;
	}
	for (int i = 0; i < numkeys; i++)
	{
		m_lastKeystate[i] = false;
	}
}

void InputHandler::FetchInput()
{
	FetchKeyboardInput();
	FetchMouseInput();

	CheckPeripherals();
	FetchControllerInput();
	FetchSteeringWheelInput();
	
	PollEvents();
}

bool InputHandler::GetKeyPressed(WKey key) const
{
	return (m_keystate[static_cast<int>(key)] && !m_lastKeystate[static_cast<int>(key)]);
}

bool InputHandler::GetKeyHold(WKey key) const
{
	return (m_keystate[static_cast<int>(key)]);
}

bool InputHandler::GetKeyReleased(WKey key) const
{
	return (!m_keystate[static_cast<int>(key)] && m_lastKeystate[static_cast<int>(key)]);
}

Vector2 InputHandler::GetMousePos() const
{
	return m_mousePosition;
}

bool InputHandler::GetMousePressed(WMouseBtn button) const
{
	return ((m_mousestate & BIT(static_cast<int>(button))) &&
		!(m_lastMousestate & BIT(static_cast<int>(button))));
}

bool InputHandler::GetMouseHold(WMouseBtn button) const
{
	return (m_mousestate & BIT(static_cast<int>(button)));
}

bool InputHandler::GetMouseReleased(WMouseBtn button) const
{
	return (!(m_mousestate & BIT(static_cast<int>(button))) &&
		(m_lastMousestate & BIT(static_cast<int>(button))));
}

Vector2 InputHandler::GetLeftStick(const Controller& controller) const
{
	return controller.leftStick;
}

Vector2 InputHandler::GetRightStick(const Controller& controller) const
{
	return controller.rightStick;
}

float InputHandler::GetLeftTrigger(const Controller& controller) const
{
	return controller.leftTrigger;
}

float InputHandler::GetRightTrigger(const Controller& controller) const
{
	return controller.rightTrigger;
}

Touchpad InputHandler::GetTouchPad() const // TODO
{
	Touchpad info;

	info.fingerOnTouchpad = false;
	info.fingerPosition = Vector2::Zero;

	return info;
}

bool InputHandler::GetControllerPresent() const
{
	for (const auto& p : m_peripherals)
		if (p.type == InputPeripheralType::Controller)
			return true;

	return false;
}

bool InputHandler::GetGamepadPressed(const InputPeripheralButtons buttonMap, WPadBtn button) const
{
	return (buttonMap.current[(int)button] && !buttonMap.last[(int)button]);
}

bool InputHandler::GetGamepadHold(const InputPeripheralButtons buttonMap, WPadBtn button) const
{
	return (buttonMap.current[(int)button]);
}

bool InputHandler::GetGamepadReleased(const InputPeripheralButtons buttonMap, WPadBtn button) const
{
	return (!buttonMap.current[(int)button] && buttonMap.last[(int)button]);
}

const InputPeripheral& InputHandler::GetInputPeripheral(const uint8 id) const
{
	return m_peripherals[static_cast<int>(id)];
}

uint8 InputHandler::GetInputPeripheralCount() const
{
	return m_peripherals.size();
}

bool InputHandler::IsInputPeripheralStillPresent(const SDL_JoystickID *joyID) const
{
	for (const auto& p : m_peripherals)
		if (p.joyID == *joyID)
			return true;
	return false;
}

void InputHandler::InitPeripheral(SDL_JoystickID joyID)
{
	InputPeripheral peripheral{};

	peripheral.joyID = joyID;
	peripheral.joystick = SDL_OpenJoystick(joyID);
	peripheral.peripheralName = SDL_GetJoystickName(peripheral.joystick);
	peripheral.usbVendorID = SDL_GetJoystickVendor(peripheral.joystick);
	peripheral.usbProductID = SDL_GetJoystickProduct(peripheral.joystick);
	peripheral.vendor = GetVendor(peripheral.usbVendorID);
	peripheral.vendorName = InputPeripheralVendor_Name[(int) peripheral.vendor];

	peripheral.buttons.current = new bool[(int) WPadBtn::Pad_Button_Count];
	peripheral.buttons.last = new bool[(int) WPadBtn::Pad_Button_Count];

	for (int i = 0; i < (int) WPadBtn::Pad_Button_Count; i++)
		peripheral.buttons.current[i] = false;
	for (int i = 0; i < (int) WPadBtn::Pad_Button_Count; i++)
		peripheral.buttons.last[i] = false;


	switch (SDL_GetJoystickType(peripheral.joystick))
	{
		case SDL_JOYSTICK_TYPE_GAMEPAD:
			peripheral.type = InputPeripheralType::Controller;

			WLog::ConsoleLog("Connected new Controller");
			std::cout << "Peripheral Info:\n";
			std::cout << "Name: " << peripheral.peripheralName << "\n";
			std::cout << "Vendor: " << peripheral.vendorName << "\n";

			peripheral.controller = InitController(peripheral.joystick);
			break;
		case SDL_JOYSTICK_TYPE_WHEEL:
			peripheral.type = InputPeripheralType::SteeringWheel;
			WLog::ConsoleLog("Connected new Steering Wheel");
			std::cout << "Peripheral Info:\n";
			std::cout << "Name: " << peripheral.peripheralName << "\n";
			std::cout << "Vendor: " << peripheral.vendorName << "\n";
			peripheral.steeringWheel = InitSteeringWheel(peripheral.joystick);
			break;

		default:
			peripheral.type = InputPeripheralType::Unknown;
			break;
	}

	m_peripherals.push_back(peripheral);
}


Controller* InputHandler::InitController(SDL_Joystick* joystick)
{
	// CLion claims that this leaks memory. It does not.
	Controller* controller = (Controller*)wNew(sizeof(Controller));

	// That whole properties thing is too scuffed, this is better im not joking.
	controller->hasRumble = SDL_RumbleJoystick(joystick, 1, 1, 1);
	controller->hasTriggerRumble = SDL_RumbleJoystickTriggers(joystick, 1, 1, 1);
	controller->hasLED = SDL_SetJoystickLED(joystick, 0x00, 0x00, 0x00);

	std::string gamepadName = SDL_GetJoystickName(joystick);

	return controller;
}

SteeringWheel* InputHandler::InitSteeringWheel(SDL_Joystick* joystick)
{
	// CLion claims that this leaks memory. It does not.
	SteeringWheel* sw = (SteeringWheel*)wNew(sizeof(SteeringWheel));

	return sw;
}

InputPeripheralVendor InputHandler::GetVendor(const uint16 id)
{
	switch (id)
	{
	case 0x054C:
		return InputPeripheralVendor::Sony;
	case 0x045E:
		return InputPeripheralVendor::Microsoft;
	case 0x057E:
		return InputPeripheralVendor::Nintendo;
	case 0x28DE: // Idk about this one though, it doesnt have a logo on the website
		return InputPeripheralVendor::Valve;
	case 0x1532: // they have two for some reason
	case 0x1689: // so i just use both, whatever...
		return InputPeripheralVendor::Razer;
	case 0x2DC8:
		return InputPeripheralVendor::EightBitDo;
	case 0x20D6:
		return InputPeripheralVendor::PowerA;
	case 0x0F0D:
		return InputPeripheralVendor::Hori;
	case 0x11C9: // From what i see, -THE- Nacon is not a USB Vendor?
	case 0x3285: // Or at least their product IDs are not public??
		return InputPeripheralVendor::Nacon;
	case 0x046D:
		return InputPeripheralVendor::Logitech;
	case 0x044F: // Two IDs again, im not a USB professional, but why
	case 0x24C6: // do some companies have two vendor IDs?
		return InputPeripheralVendor::Thrustmaster;
	case 0x3767: // idk if its 3767 or 0x3767, if someone complains then its just 3767
		return InputPeripheralVendor::Fanatech;
	case 0x10F5:
		return InputPeripheralVendor::Turtlebeach;
	default:
		return InputPeripheralVendor::Unknown;
	}
}

void InputHandler::FetchKeyboardInput()
{
	// Swap keystate buffers
	bool* temp = m_lastKeystate;
	m_lastKeystate = m_keystate;
	m_keystate = temp;

	// Update keystates
	int numkeys;
	bool* sdlkeyState = const_cast<bool*>(SDL_GetKeyboardState(&numkeys));
	std::memcpy(m_keystate, sdlkeyState, sizeof(bool) * numkeys);
}

void InputHandler::FetchMouseInput()
{
	// It's ok, since im not swapping buffer unlike above
	m_lastMousestate = m_mousestate;

	float mouseY, mouseX;
	m_mousestate = SDL_GetMouseState(&mouseX, &mouseY);
	m_mousePosition = { mouseX, mouseY };
}

void InputHandler::FetchControllerInput()
{
	for (auto& device : m_peripherals)
	{
		if (device.type != InputPeripheralType::Controller)
			continue;

		FetchPadButtonInput(device);

		device.controller->leftStick.x = Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, 0));
		device.controller->leftStick.y = Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, 1));
		device.controller->leftTrigger = FloatRange1_1To0_1(Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, 2)));

		device.controller->rightStick.x = Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, 3));
		device.controller->rightStick.y = Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, 4));
		device.controller->rightTrigger = FloatRange1_1To0_1(Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, 5)));


		if (Vector2::Magnitude(device.controller->leftStick) < EngineSettings::inputStickDeadzone)
			device.controller->leftStick = Vector2::Zero;
		if (Vector2::Magnitude(device.controller->rightStick) < EngineSettings::inputStickDeadzone)
			device.controller->rightStick = Vector2::Zero;
	}

}

void InputHandler::FetchSteeringWheelInput()
{
	for (auto& device : m_peripherals)
	{
		if (device.type != InputPeripheralType::SteeringWheel)
			continue;

		// I don't know whether this will work on anything other
		// than the G920, and i don't have enough money to find out.
		device.steeringWheel->wheel = Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, SDL_GAMEPAD_AXIS_LEFTX)); // wheel
		device.steeringWheel->throttle = FloatRange1_1To0_1(-Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, SDL_GAMEPAD_AXIS_LEFTY))); // throttle
		device.steeringWheel->brake = FloatRange1_1To0_1(-Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, SDL_GAMEPAD_AXIS_RIGHTX))); // brake
		device.steeringWheel->clutch = FloatRange1_1To0_1(-Sint16ToFloat(SDL_GetJoystickAxis(device.joystick, SDL_GAMEPAD_AXIS_RIGHTY))); // clutch



		FetchPadButtonInput(device);
	}
}

void InputHandler::FetchPadButtonInput(InputPeripheral& device)
{

	bool* temp = device.buttons.last;
	device.buttons.last = device.buttons.current;
	device.buttons.current = temp;

	bool* joyButtons = (bool*)wNew(SDL_GAMEPAD_BUTTON_COUNT);
	for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT - 1; i++)
		joyButtons[i] = SDL_GetJoystickButton(device.joystick, i);

	// this is for adding mapping new controllers
	//for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT - 1; i++)
	//	if (joyButtons[i])
	//		WLog::ConsoleLog(std::format("Button {} is being pressed", i));

	switch (device.vendor)
	{
	case InputPeripheralVendor::Sony:
		FetchPadButtonInput_Sony(device, joyButtons);
		break;
	case InputPeripheralVendor::Microsoft:
		FetchPadButtonInput_Microsoft(device, joyButtons);
		break;
	case InputPeripheralVendor::Logitech:
		FetchPadButtonInput_Logitech(device, joyButtons);
		break;

	default:
		break;
	}

	wFree(joyButtons);
}

void InputHandler::FetchPadButtonInput_Sony(const InputPeripheral& device, const bool* joyButtons)
{
	device.buttons.current[(int)WPadBtn::South] = joyButtons[0];
	device.buttons.current[(int)WPadBtn::East] = joyButtons[1];
	device.buttons.current[(int)WPadBtn::West] = joyButtons[3];
	device.buttons.current[(int)WPadBtn::North] = joyButtons[2];

	device.buttons.current[(int)WPadBtn::Select] = joyButtons[8];
	device.buttons.current[(int)WPadBtn::Start] = joyButtons[9];

	device.buttons.current[(int)WPadBtn::LS] = joyButtons[11];
	device.buttons.current[(int)WPadBtn::RS] = joyButtons[12];
	device.buttons.current[(int)WPadBtn::LB] = joyButtons[4];
	device.buttons.current[(int)WPadBtn::RB] = joyButtons[5];
	device.buttons.current[(int)WPadBtn::LT] = joyButtons[6];
	device.buttons.current[(int)WPadBtn::RT] = joyButtons[7];

	const auto& hat = SDL_GetJoystickHat(device.joystick, 0);
	device.buttons.current[(int)WPadBtn::Up] =		hat == SDL_HAT_UP	 || hat == SDL_HAT_LEFTUP	|| hat == SDL_HAT_RIGHTUP;
	device.buttons.current[(int)WPadBtn::Down] =	hat == SDL_HAT_DOWN	 || hat == SDL_HAT_LEFTDOWN	|| hat == SDL_HAT_RIGHTDOWN;
	device.buttons.current[(int)WPadBtn::Left] =	hat == SDL_HAT_LEFT	 || hat == SDL_HAT_LEFTUP	|| hat == SDL_HAT_LEFTDOWN;
	device.buttons.current[(int)WPadBtn::Right] =	hat == SDL_HAT_RIGHT || hat == SDL_HAT_RIGHTUP	|| hat == SDL_HAT_RIGHTDOWN;
}
void InputHandler::FetchPadButtonInput_Microsoft(const InputPeripheral& device, const bool* joyButtons)
{
	device.buttons.current[(int)WPadBtn::South] = joyButtons[0];
	device.buttons.current[(int)WPadBtn::East] = joyButtons[1];
	device.buttons.current[(int)WPadBtn::West] = joyButtons[2];
	device.buttons.current[(int)WPadBtn::North] = joyButtons[3];

	device.buttons.current[(int)WPadBtn::Select] = joyButtons[6];
	device.buttons.current[(int)WPadBtn::Start] = joyButtons[7];

	device.buttons.current[(int)WPadBtn::LS] = joyButtons[9];
	device.buttons.current[(int)WPadBtn::RS] = joyButtons[10];
	device.buttons.current[(int)WPadBtn::LB] = joyButtons[4];
	device.buttons.current[(int)WPadBtn::RB] = joyButtons[5];
	device.buttons.current[(int)WPadBtn::LT] = SDL_GetJoystickAxis(device.joystick, 2) > EngineSettings::triggerBoolPressThreshold;
	device.buttons.current[(int)WPadBtn::RT] = SDL_GetJoystickAxis(device.joystick, 5) > EngineSettings::triggerBoolPressThreshold;

	const auto& hat = SDL_GetJoystickHat(device.joystick, 0);
	device.buttons.current[(int)WPadBtn::Up] =		hat == SDL_HAT_UP	 || hat == SDL_HAT_LEFTUP	|| hat == SDL_HAT_RIGHTUP;
	device.buttons.current[(int)WPadBtn::Down] =	hat == SDL_HAT_DOWN	 || hat == SDL_HAT_LEFTDOWN	|| hat == SDL_HAT_RIGHTDOWN;
	device.buttons.current[(int)WPadBtn::Left] =	hat == SDL_HAT_LEFT	 || hat == SDL_HAT_LEFTUP	|| hat == SDL_HAT_LEFTDOWN;
	device.buttons.current[(int)WPadBtn::Right] =	hat == SDL_HAT_RIGHT || hat == SDL_HAT_RIGHTUP	|| hat == SDL_HAT_RIGHTDOWN;
}
void InputHandler::FetchPadButtonInput_Logitech(const InputPeripheral& device, const bool* joyButtons)
{
	// the steering wheel, not their controllers

	device.buttons.current[(int)WPadBtn::South] = joyButtons[0];
	device.buttons.current[(int)WPadBtn::East] = joyButtons[1];
	device.buttons.current[(int)WPadBtn::West] = joyButtons[2];
	device.buttons.current[(int)WPadBtn::North] = joyButtons[3];

	device.buttons.current[(int)WPadBtn::Select] = joyButtons[7];
	device.buttons.current[(int)WPadBtn::Start] = joyButtons[6];

	// this are the rsb/lsb buttons
	device.buttons.current[(int)WPadBtn::LB] = joyButtons[9];
	device.buttons.current[(int)WPadBtn::RB] = joyButtons[8];

	const auto& hat = SDL_GetJoystickHat(device.joystick, 0);
	device.buttons.current[(int)WPadBtn::Up] =		hat == SDL_HAT_UP	 || hat == SDL_HAT_LEFTUP	|| hat == SDL_HAT_RIGHTUP;
	device.buttons.current[(int)WPadBtn::Down] =	hat == SDL_HAT_DOWN	 || hat == SDL_HAT_LEFTDOWN	|| hat == SDL_HAT_RIGHTDOWN;
	device.buttons.current[(int)WPadBtn::Left] =	hat == SDL_HAT_LEFT	 || hat == SDL_HAT_LEFTUP	|| hat == SDL_HAT_LEFTDOWN;
	device.buttons.current[(int)WPadBtn::Right] =	hat == SDL_HAT_RIGHT || hat == SDL_HAT_RIGHTUP	|| hat == SDL_HAT_RIGHTDOWN;

	device.steeringWheel->sequentialShifterState = 0;

	// I bet someone on reddit can make an ugly one-liner out of this
	if (joyButtons[5])
		device.steeringWheel->sequentialShifterState = 1;
	if (joyButtons[4])
		device.steeringWheel->sequentialShifterState = 2;
	if (joyButtons[5] && joyButtons[4])
		device.steeringWheel->sequentialShifterState = 3;
}

void InputHandler::FetchPadButtonInput_Turtlebeach(const InputPeripheral &device, const bool *joyButtons)
{
	// it turns out that i use this controller on linux lmao
}

void InputHandler::CheckPeripherals()
{
	int count;
	SDL_JoystickID* joyID = SDL_GetJoysticks(&count);

	if (joyID == nullptr)
		return;

	CheckMissingPeripherals(count, joyID);
	for (int i = 0; i < count; i++)
	{
		bool isPresent = false;
		for (const auto& peripheral : m_peripherals)
		{
			if (joyID[i] == peripheral.joyID)
			{
				isPresent = true;
				break;
			}
		}

		if (isPresent)
			continue;

		InitPeripheral(joyID[i]);
	}

	SDL_free(joyID);
}

void InputHandler::CheckMissingPeripherals(int count, const SDL_JoystickID* joyID)
{
	bool found = false;
	// iterate backwards so erase doesn't break indexing
	for (int i = (int)m_peripherals.size() - 1; i >= 0; i--)
	{
		found = false;
		for (int j = 0; j < count; j++)
		{
			if (joyID[j] == m_peripherals[i].joyID)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			WLog::SetConsoleInfo();
			WLog::ConsoleLog(std::format("Unplugged peripheral: {}", m_peripherals[i].peripheralName));

			m_peripherals.erase(m_peripherals.begin() + i);
		}
	}
}

void InputHandler::PollEvents() const
{
	while (SDL_PollEvent(m_inputEvent))
	{
		ImGui_ImplSDL3_ProcessEvent(m_inputEvent);
		if (m_inputEvent->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		{
			CoreSystems::ShutdownGame();
		}
	}
}

// These reset functions exist to prevent unwanted pressed when a controller
// is registered and placed over uncleared memory.
void InputHandler::ResetController(Controller* controller)
{
	controller->leftStick = Vector2::Zero;
	controller->rightStick = Vector2::Zero;
	controller->leftTrigger = 0.0f;
	controller->rightTrigger = 0.0f;
}

void InputHandler::ResetSteeringWheel(SteeringWheel* wheel)
{
	wheel->throttle = 0.0f;
	wheel->brake = 0.0f;
	wheel->clutch = 0.0f;
	wheel->wheel = 0.0f;

	wheel->hShifterPosition = 0;
	wheel->sequentialShifterState = 0;
	wheel->lastSequentialShifterState = 0;
	wheel->parkingBrakeState = false;
}
