#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>
#include <Engine/Types/Input/KeyCodes.h>
#include <Engine/Types/Input/InputDevice.h>

namespace WEngine
{
	enum PressType
	{
		Press,
		Hold,
		Release
	};

	class InputHandler;
	class InputComponent : public Component
	{
	public:
		InputComponent(Entity* e);

	private:
		InputPeripheral* m_selectedPeripheral = nullptr;
	public:
		void SelectFirstController();
		void SelectFirstWheel();
		bool IsControllerSelected();

		[[nodiscard]] static bool GetActionInput(WKey key); // Defaults presstype to hold
		[[nodiscard]] static bool GetActionInput(WKey key, PressType pressType);
		[[nodiscard]] static bool GetActionInput(WMouseBtn button); // Defaults presstype to hold
		[[nodiscard]] static bool GetActionInput(WMouseBtn button, PressType pressType);
		[[nodiscard]] bool GetActionInput(WPadBtn button); // Defaults presstype to hold
		[[nodiscard]] bool GetActionInput(WPadBtn button, PressType pressType);

		[[nodiscard]] static Vector2 GetMousePosition();

		[[nodiscard]] Vector2 GetLeftStick();
		[[nodiscard]] Vector2 GetRightStick();
		[[nodiscard]] float32 GetLeftTrigger();
		[[nodiscard]] float32 GetRightTrigger();
		[[nodiscard]] static bool GetControllerPresent();

		COMP_HASH(0x24c07efdaf193f80)
	};

}

