#pragma once

#include <WidroloFramework.h>

class Freecam : public WEngine::Component
{
public:
    Freecam(WEngine::Entity* e);

public:
    void Awake(WEngine::ComponentArgs ca) override;
    void Start() override;
    void Tick(float32 dt) override;

private:
    float32 m_speed;
    float32 m_yaw;
    float32 m_pitch;

    bool m_focused;

    WEngine::Vector2 m_oldMousePos;

    COMP_HASH(0xe6321c612fcb459b)
};

