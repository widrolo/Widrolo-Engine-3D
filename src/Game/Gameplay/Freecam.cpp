#include "Freecam.h"

#include "Engine/Core/Handlers/Input.h"
#include "Engine/Core/Handlers/RenderHandler.h"
#include "Engine/Core/System/Haptic.h"
#include "Engine/Util/TimeAnalysis.h"
#include "Engine/Types/Audio/Echo/Handles.h"
#include "Engine/Core/System/Echo.h"

Echo::AudioBusHandle bus;

Freecam::Freecam()
{
    m_speed = 5.0f;
    m_trans = WEngine::Transform::Zero;
    m_yaw = 120.0f;
    m_pitch = -20.0f;
    m_trans.position.x = -8;
    m_trans.position.y = 3;
    m_trans.position.z = -6;

    Echo::BusDesc busDesc{};
    busDesc.name = "Freecam";
    bus = Echo::CreateBus(busDesc);
}

void Freecam::Tick(float32 dt)
{
    WEngine::TimeSample sample("Freecam::Tick");
    static bool firstFrame = true;
    if (firstFrame)
    {
        firstFrame = false;
        return;
    }

    if (dt > 10.0f)
        return;

    float32 speed = m_speed * dt;

    WEngine::Vector2 move = Input::GetVector("camMove");
    WEngine::Vector2 look = Input::GetVector("camLook");

    if (Input::GetAction("camSpeed", PressType::Hold))
        speed *= 2.0f;

    WEngine::Vector3 moveForward = m_trans.Forward();
    moveForward.y = -moveForward.y;

    m_trans.position = m_trans.position + moveForward * move.y * speed + m_trans.Right() * move.x * speed;


    if (Input::GetAction("camHigher", PressType::Hold))
        m_trans.position.y += speed;
    if (Input::GetAction("camLower", PressType::Hold))
        m_trans.position.y -= speed;

    m_yaw += look.x;
    m_pitch +=  look.y / 1.5f;

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    m_trans.rotation = WEngine::Quaternion::EulerToQuaternion({glm::radians(m_pitch), glm::radians(m_yaw), 0.0f});
}

void Freecam::UploadCamera()
{
    WEngine::CoreSystems::GetRenderHandler()->UpdateCamera(m_trans);
}
