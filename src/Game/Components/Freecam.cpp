#include "Freecam.h"

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

    if (input->GetActionInput(WKey::E))
        entity->transform.position = entity->transform.position + entity->transform.Up() * m_speed * dt;
    if (input->GetActionInput(WKey::Q))
        entity->transform.position = entity->transform.position - entity->transform.Up() * m_speed * dt;


    auto newMousePos = input->GetMousePosition();

    WEngine::Vector2 mouseDelta = m_oldMousePos - (newMousePos - WEngine::Vector2{1920/2, 1080/2});

    entity->transform.rotation.y = entity->transform.rotation.y - (mouseDelta.x / 10);
    entity->transform.rotation.x = entity->transform.rotation.x + (mouseDelta.y / 10);

    m_oldMousePos = newMousePos - WEngine::Vector2{1920/2, 1080/2};

    return;

    if (input->GetActionInput(WKey::LEFT))
        entity->transform.rotation.y = entity->transform.rotation.y - (dt * 20.0f);
    if (input->GetActionInput(WKey::RIGHT))
        entity->transform.rotation.y = entity->transform.rotation.y + (dt * 20.0f);

    if (input->GetActionInput(WKey::UP))
        entity->transform.rotation.x = entity->transform.rotation.x + (dt * 20.0f);
    if (input->GetActionInput(WKey::DOWN))
        entity->transform.rotation.x = entity->transform.rotation.x - (dt * 20.0f);

}
