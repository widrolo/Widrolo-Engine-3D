#include "Freecam.h"

#include "Engine/Core/Handlers/InputHandler.h"

const bool enabledByDefault = false;

REGISTER_COMPONENT(Freecam)

Freecam::Freecam(WEngine::Entity *e)
{
    COMP_SETUP("Freecam")
}

void Freecam::Awake(WEngine::ComponentArgs ca)
{
    m_speed = 3.0f;
}

void Freecam::Start()
{
    m_oldMousePos = input->GetMousePosition();
    WEngine::CoreSystems::GetInputHandler()->SetMouseRelativeMode(enabledByDefault);
    m_focused = enabledByDefault;
}

void Freecam::Tick(float32 dt)
{
    static bool firstFrame = true;
    if (firstFrame)
    {
        firstFrame = false;
        return;
    }

    if (dt > 10.0f)
        return;

    float32 speed = m_speed * dt;

    if (input->GetActionInput(WKey::CONTROL))
        speed *= 2.0f;

    if (input->GetActionInput(WKey::W))
        entity->transform.position = entity->transform.position + entity->transform.Forward() * speed;
    if (input->GetActionInput(WKey::S))
        entity->transform.position = entity->transform.position - entity->transform.Forward() * speed;

    if (input->GetActionInput(WKey::A))
        entity->transform.position = entity->transform.position - entity->transform.Right() * speed;
    if (input->GetActionInput(WKey::D))
        entity->transform.position = entity->transform.position + entity->transform.Right() * speed;

    if (input->GetActionInput(WKey::SPACE))
        entity->transform.position = entity->transform.position + entity->transform.Up() * speed;
    if (input->GetActionInput(WKey::SHIFT))
        entity->transform.position = entity->transform.position - entity->transform.Up() * speed;


    if (input->GetActionInput(WKey::DEBUG5, WEngine::Press))
    {
        m_focused = !m_focused;
        WEngine::CoreSystems::GetInputHandler()->SetMouseRelativeMode(m_focused);
        firstFrame = true; // hacky solution, but it works; so meh
        return;
    }

    if (!m_focused)
        return;

    WEngine::Vector2 mouseDelta = input->GetMousePosition();

    m_yaw += (mouseDelta.x / 3);
    m_pitch -=  (mouseDelta.y / 3);

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    entity->transform.rotation.y = m_yaw;
    entity->transform.rotation.x = m_pitch;
}
