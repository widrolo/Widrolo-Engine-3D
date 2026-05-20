#include "Freecam.h"

#include "Engine/Core/Handlers/InputHandler.h"

REGISTER_COMPONENT(Freecam)

Freecam::Freecam(WEngine::Entity *e)
{
    COMP_SETUP("Freecam")
}

void Freecam::Awake(WEngine::ComponentArgs ca)
{
    m_speed = 0.4f;
}

void Freecam::Start()
{
    m_oldMousePos = input->GetMousePosition();
}

void Freecam::Tick(float32 dt)
{
    if (dt > 10.0f)
        return;


    if (input->GetActionInput(WKey::W))
        entity->transform.position = entity->transform.position + entity->transform.Forward() * m_speed * dt;
    if (input->GetActionInput(WKey::S))
        entity->transform.position = entity->transform.position - entity->transform.Forward() * m_speed * dt;

    if (input->GetActionInput(WKey::A))
        entity->transform.position = entity->transform.position - entity->transform.Right() * m_speed * dt;
    if (input->GetActionInput(WKey::D))
        entity->transform.position = entity->transform.position + entity->transform.Right() * m_speed * dt;

    if (input->GetActionInput(WKey::SPACE))
        entity->transform.position = entity->transform.position + entity->transform.Up() * m_speed * dt;
    if (input->GetActionInput(WKey::SHIFT))
        entity->transform.position = entity->transform.position - entity->transform.Up() * m_speed * dt;


    auto newMousePos = input->GetMousePosition();
    WEngine::CoreSystems::GetInputHandler()->SetMousePos(EngineSettings::resolution / 2);

    WEngine::Vector2 mouseDelta = newMousePos - EngineSettings::resolution / 2;

    m_yaw += (mouseDelta.x / 2);
    m_pitch -=  (mouseDelta.y / 2);

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    entity->transform.rotation.y = m_yaw;
    entity->transform.rotation.x = m_pitch;
}
