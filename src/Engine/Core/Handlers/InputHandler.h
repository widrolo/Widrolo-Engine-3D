#pragma once

#include <SDL3/SDL.h>
#include <cstring>
#include <algorithm>
#include <Engine/WTL/vector.h>

#include <Engine/Math/Vector.h>
#include <../../Types/Input/KeyCodes.h>
#include <Engine/Types/CommonTypes.h>
#include <../../Types/Input/InputDevice.h>

namespace WEngine
{
	/**
	 * The InputHandler class is responsible for handling input events from various devices such as the keyboard, mouse, gamepads, and steering wheels.
	 */
	class InputHandler
	{
	public:
		/**
		 * Constructs an instance of the InputHandler class. Initializes the key states and opens joysticks if any are connected.
		 */
		InputHandler();
	private:
		SDL_Event* m_inputEvent = new SDL_Event;
		bool* m_keystate;
		bool* m_lastKeystate;

		Vector2 m_mousePosition;
		uint32 m_mousestate{};
		uint32 m_lastMousestate{};

		wtl::vector<InputPeripheral> m_peripherals;

	public:
		/**
		 * Fetches input from keyboard, mouse, controller, and steering wheel.
		 * Also checks for newly connected or disconnected peripherals.
		 */
		void FetchInput();
		/**
		 * Checks if a specific key has been pressed in the current frame.
		 * @param key The key to check.
		 * @return True if the key was just pressed, false otherwise.
		 */
		[[nodiscard]] bool GetKeyPressed(WKey key) const;
		/**
		 * Checks if a specific key has been pressed.
		 * @param key The key to check.
		 * @return True if the key is held down, false otherwise.
		 * @note This is not mutually Exclusive with GetKeyPressed; if a key is being pressed on the first frame, then
		 * this function will also report as true.
		 */
		[[nodiscard]] bool GetKeyHold(WKey key) const;
		/**
		 * Checks if a keyboard key was released in this frame.
		 * @param key The key to check.
		 * @return True if the key was released, false otherwise.
		 */
		[[nodiscard]] bool GetKeyReleased(WKey key) const;
		/**
		 * Retrieves the current mouse position as a Vector2.
		 * @return The current mouse position.
		 */
		[[nodiscard]] Vector2 GetMousePos() const;
		/**
		 * Checks if a specific mouse button is pressed during this frame and was not pressed in the previous frame.
		 * @param button The mouse button to check for a press event.
		 * @return True if the button is pressed, false otherwise.
		 */
		[[nodiscard]] bool GetMousePressed(WMouseBtn button) const;
		/**
		 * Checks if a specific mouse button is pressed.
		 * @param button The mouse button to check for a hold event.
		 * @return True if the specified mouse button is being held down, false otherwise.
		 */
		[[nodiscard]] bool GetMouseHold(WMouseBtn button) const;
		/**
		 * Checks if a specified mouse button has been released since the last input update.
		 * @param button The mouse button to check for release.
		 * @return True if the specified mouse button was released, false otherwise.
		 * @note This is not mutually Exclusive with GetMousePressed; if a button is being pressed on the first frame,
		 * then this function will also report as true.
		 */
		[[nodiscard]] bool GetMouseReleased(WMouseBtn button) const;
		/**
		 * Fetches the current position of the left stick on a controller.
		 * @param controller The controller object from which to get the left stick position.
		 * @return A Vector2 representing the current position of the left stick.
		 */
		[[nodiscard]] Vector2 GetLeftStick(const Controller &controller) const;
		/**
		 * Retrieves the current position of a controller's right stick.
		 * @param controller The gamepad from which to retrieve the right stick position.
		 * @return A Vector2 representing the current position of the right stick on the given gamepad.
		 */
		[[nodiscard]] Vector2 GetRightStick(const Controller &controller) const;
		/**
		 * Retrieves the current pressure value of the left trigger on a specified controller.
		 * @param controller The input controller to check for left trigger input.
		 * @return A float representing the current pressure value of the left trigger, ranging from 0.0f to 1.0f.
		 */
		[[nodiscard]] float GetLeftTrigger(const Controller &controller) const;
		/**
		 * Retrieves the current pressure value of the right trigger on a specified controller.
		 * @param controller The input controller to check for right trigger input.
		 * @return A float representing the current pressure value of the right trigger, ranging from 0.0f to 1.0f.
		 */
		[[nodiscard]] float GetRightTrigger(const Controller &controller) const;
		/**
		 * @deprecated based on the old version, this will soon be replaced.
		 */
		[[deprecated]][[nodiscard]] Touchpad GetTouchPad() const;
		/**
		 * Checks whether a controller is currently connected.
		 * @return True if a controller is connected.
		 */
		[[nodiscard]] bool GetControllerPresent() const;
		/**
		 * Checks whether a button was pressed this frame.
		 * @param buttonMap Button map of the controller to be checked.
		 * @param button Button to be compared against.
		 * @return True if the button on the controller was pressed.
		 */
		[[nodiscard]] bool GetGamepadPressed(InputPeripheralButtons buttonMap, WPadBtn button) const;
		/**
		 * Checks whether a button was pressed.
		 * @param buttonMap Button map of the controller to be checked.
		 * @param button Button to be compared against.
		 * @return True if the button on the controller was pressed.
		 */
		[[nodiscard]] bool GetGamepadHold(InputPeripheralButtons buttonMap, WPadBtn button) const;
		/**
		 * Checks whether a button was released this frame.
		 * @param buttonMap Button map of the controller to be checked.
		 * @param button Button to be compared against.
		 * @return True if the button on the controller was pressed.
		 * @note This is not mutually Exclusive with GetGamepadPressed; if a button is being pressed on the first frame,
		 * then this function will also report as true.
		 */
		[[nodiscard]] bool GetGamepadReleased(InputPeripheralButtons buttonMap, WPadBtn button) const;
		/**
		 * Retrieves an input peripheral based on an ID.
		 * @param id ID of the input peripheral.
		 * @return A constant reference to the peripheral, or a juicy SIGSEGV.
		 * @note I was not joking about the SIGSEGV thing.
		 */
		[[nodiscard]] const InputPeripheral& GetInputPeripheral(uint8 id) const;
		/**
		 * @return The number of connected peripherals.
		 */
		[[nodiscard]] uint8 GetInputPeripheralCount() const;

		/**
		 * Checks whether the input peripheral is still present based on the SDL JoyID
		 * @param joyID The known JoyID of the peripheral.
		 * @return True if It's still connected.
		 */
		[[nodiscard]] bool IsInputPeripheralStillPresent(const SDL_JoystickID* joyID) const;
	private:
		void InitPeripheral(SDL_JoystickID joyID);
		static Controller* InitController(SDL_Joystick* joystick);
		static SteeringWheel* InitSteeringWheel(SDL_Joystick* joystick);
		static InputPeripheralVendor GetVendor(uint16 id) ;

		void FetchKeyboardInput();
		void FetchMouseInput();
		void FetchControllerInput();
		void FetchSteeringWheelInput();
		static void FetchPadButtonInput(InputPeripheral& device);

		// these are not all, since i dont have them all
		static void FetchPadButtonInput_Sony(const InputPeripheral& device, const bool* joyButtons);
		static void FetchPadButtonInput_Microsoft(const InputPeripheral& device, const bool* joyButtons);
		static void FetchPadButtonInput_Logitech(const InputPeripheral& device, const bool* joyButtons);
		static void FetchPadButtonInput_Turtlebeach(const InputPeripheral& device, const bool* joyButtons);

		void CheckPeripherals();
		void CheckMissingPeripherals(int count, const SDL_JoystickID* joyID);
		void PollEvents() const;

		static void ResetController(Controller* controller);
		static void ResetSteeringWheel(SteeringWheel* wheel);
	};
}
