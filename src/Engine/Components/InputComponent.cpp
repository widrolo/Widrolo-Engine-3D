#include "InputComponent.h"

#include <Engine/Core/Handlers/InputHandler.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/World/Entity.h>

using namespace WEngine;

REGISTER_COMPONENT(InputComponent)

InputComponent::InputComponent(Entity* e)
{
	COMP_SETUP("InputHandler")
}

void InputComponent::SelectFirstController()
{
	const int count = CoreSystems::GetInputHandler()->GetInputPeripheralCount();

	for (int i = 0; i < count; i++)
	{
		auto ip = CoreSystems::GetInputHandler()->GetInputPeripheral(i);
		if (ip.type == InputPeripheralType::Controller)
		{
			m_selectedPeripheral = &ip;
			return;
		}
	}
}
void InputComponent::SelectFirstWheel()
{

}

bool InputComponent::IsControllerSelected()
{
	return m_selectedPeripheral != nullptr;
}

bool InputComponent::GetActionInput(const WKey key)
{
	return CoreSystems::GetInputHandler()->GetKeyHold(key);
}

bool InputComponent::GetActionInput(const WKey key, const PressType pressType)
{
	switch (pressType)
	{
	case Press:
		return CoreSystems::GetInputHandler()->GetKeyPressed(key);
	case Hold:
		return CoreSystems::GetInputHandler()->GetKeyHold(key);
	case Release:
		return CoreSystems::GetInputHandler()->GetKeyReleased(key);
	default:
		return false;
	}
}

bool InputComponent::GetActionInput(const WMouseBtn button)
{
	return CoreSystems::GetInputHandler()->GetMouseHold(button);
}

bool InputComponent::GetActionInput(const WMouseBtn button, const PressType pressType)
{
	switch (pressType)
	{
	case Press:
		return CoreSystems::GetInputHandler()->GetMousePressed(button);
	case Hold:
		return CoreSystems::GetInputHandler()->GetMouseHold(button);
	case Release:
		return CoreSystems::GetInputHandler()->GetMouseReleased(button);
	default:
		return false;
	}
}

bool InputComponent::GetActionInput(const WPadBtn button)
{
	SelectFirstController();
	if (m_selectedPeripheral == nullptr)
		return false;
	return CoreSystems::GetInputHandler()->GetGamepadHold(m_selectedPeripheral->buttons, button);
}

bool InputComponent::GetActionInput(const WPadBtn button, const PressType pressType)
{
	SelectFirstController();
	if (m_selectedPeripheral == nullptr)
		return false;
	switch (pressType)
	{
	case Press:
		return CoreSystems::GetInputHandler()->GetGamepadPressed(m_selectedPeripheral->buttons, button);
	case Hold:
		return CoreSystems::GetInputHandler()->GetGamepadHold(m_selectedPeripheral->buttons, button);
	case Release:
		return CoreSystems::GetInputHandler()->GetGamepadReleased(m_selectedPeripheral->buttons, button);
	default:
		return false;
	}
}

Vector2 InputComponent::GetMousePosition()
{
	return CoreSystems::GetInputHandler()->GetMousePos();
}

Vector2 InputComponent::GetLeftStick()
{
	SelectFirstController();
	if (m_selectedPeripheral == nullptr)
		return Vector2::Zero;
	return CoreSystems::GetInputHandler()->GetLeftStick(*m_selectedPeripheral->controller);
}

Vector2 InputComponent::GetRightStick()
{
	SelectFirstController();
	if (m_selectedPeripheral == nullptr)
		return Vector2::Zero;
	return CoreSystems::GetInputHandler()->GetRightStick(*m_selectedPeripheral->controller);
}

float32 InputComponent::GetLeftTrigger()
{
	SelectFirstController();
	if (m_selectedPeripheral == nullptr)
		return 0.0f;
	return CoreSystems::GetInputHandler()->GetLeftTrigger(*m_selectedPeripheral->controller);
}

float32 InputComponent::GetRightTrigger()
{
	SelectFirstController();
	if (m_selectedPeripheral == nullptr)
		return 0.0f;
	return CoreSystems::GetInputHandler()->GetRightTrigger(*m_selectedPeripheral->controller);
}


bool InputComponent::GetControllerPresent()
{
	return CoreSystems::GetInputHandler()->GetControllerPresent();
}
