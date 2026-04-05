#include "PeripheralWidget.h"

#include <Engine/Core/Handlers/InputHandler.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Types/CoreSystems.h>

using namespace WEngine;

void PeripheralWidget::Setup()
{
    m_widgetName = "Peripherals";
    const auto repo = CoreSystems::GetAssetRepo();

    //SpriteAssetMission mission{};
    //mission.name = "../EditorData/Images/Buttons/Buttons";
    //repo->GetAsset(mission);
    //m_faceButtonTexture = (ImTextureID)mission.sprite;
//
    //mission.name = "../EditorData/Images/Buttons/DPad";
    //repo->GetAsset(mission);
    //m_dPadTexture = (ImTextureID)mission.sprite;
//
    //mission.name = "../EditorData/Images/Buttons/Shoulder";
    //repo->GetAsset(mission);
    //m_shoulderTexture = (ImTextureID)mission.sprite;
//
    //mission.name = "../EditorData/Images/Buttons/Sequential";
    //repo->GetAsset(mission);
    //m_sequentiaTexture = (ImTextureID)mission.sprite;
//
    //mission.name = "../EditorData/Images/Buttons/HShifter";
    //repo->GetAsset(mission);
    //m_hShifterTexture= (ImTextureID)mission.sprite;
}

void PeripheralWidget::RenderInternal()
{
    SetSize({300, 350});

    uint8 count = CoreSystems::GetInputHandler()->GetInputPeripheralCount();


    for (uint8 i = 0; i < count; ++i)
    {
        auto p = CoreSystems::GetInputHandler()->GetInputPeripheral(i);
        if (p.type == InputPeripheralType::Controller)
            RenderController(p);
        if (p.type == InputPeripheralType::SteeringWheel)
            RenderSteeringWheel(p);
    }
}

void PeripheralWidget::RenderController(const InputPeripheral& p) const
{
    ImGuiSliderFlags flags = ImGuiSliderFlags_NoInput;
    ImGui::SeparatorText("Controller");
    ImGui::Text("%s", p.peripheralName.c_str());

    float triggers[2] = { p.controller->leftTrigger, p.controller->rightTrigger };
    float lJoy[2] = { p.controller->leftStick.x, p.controller->leftStick.y };
    float rJoy[2] = { p.controller->rightStick.x, p.controller->rightStick.y };
    ImGui::SliderFloat2("L2/R2", triggers, 0.0f, 1.0f, "%.3f", flags);
    ImGui::SliderFloat2("Left Joy (X/Y)", lJoy, -1.0f, 1.0f, "%.3f", flags);
    ImGui::SliderFloat2("Right Joy (X/Y)", rJoy, -1.0f, 1.0f, "%.3f", flags);

    //RenderFaceButtons(p.buttons);
    //RenderDPad(p.buttons);
    //ImGui::SameLine();
    //RenderShoulder(p.buttons);
}
void PeripheralWidget::RenderSteeringWheel(const InputPeripheral& p) const
{
    ImGuiSliderFlags flags = ImGuiSliderFlags_NoInput;
    ImGui::SeparatorText("Steering Wheel");
    ImGui::Text("%s", p.peripheralName.c_str());
    ImGui::SliderFloat("Wheel", &p.steeringWheel->wheel, -1.0f, 1.0f, "%.3f", flags);
    ImGui::SliderFloat("Throttle", &p.steeringWheel->throttle, 0.0f, 1.0f, "%.3f", flags);
    ImGui::SliderFloat("Brake", &p.steeringWheel->brake, 0.0f, 1.0f, "%.3f", flags);
    ImGui::SliderFloat("Clutch", &p.steeringWheel->clutch, 0.0f, 1.0f, "%.3f", flags);

    //RenderFaceButtons(p.buttons);
    //RenderDPad(p.buttons);
    //ImGui::SameLine();
    //RenderSequential(p.steeringWheel->sequentialShifterState);
    //ImGui::SameLine();
    //RenderHShifter(p.steeringWheel->hShifterPosition); // i know i dont have one, but it should display something???
}

void PeripheralWidget::RenderFaceButtons(const InputPeripheralButtons &buttons) const
{
    const bool pressed[4] = {
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::South),
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::East),
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::West),
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::North),
    };

    for (int i = 0; i < 4; ++i) {
        DrawButton(m_faceButtonTexture, 1./4, 1./6, pressed[i], i);
        ImGui::SameLine(); // this can be here too since next up is DPad
    }
}

void PeripheralWidget::RenderDPad(const InputPeripheralButtons &buttons) const
{
    //std::cout << buttons.current[SDL_GAMEPAD_BUTTON_DPAD_UP] << "\n";
    if (CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::Up))
        DrawButton(m_dPadTexture, 1./5, 1, 0, 1);
    else if (CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::Right))
        DrawButton(m_dPadTexture, 1./5, 1, 0, 2);
    else if (CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::Down))
        DrawButton(m_dPadTexture, 1./5, 1, 0, 3);
    else if (CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::Left))
        DrawButton(m_dPadTexture, 1./5, 1, 0, 4);
    else
        DrawButton(m_dPadTexture, 1./5, 1, 0, 0);

}

void PeripheralWidget::RenderShoulder(const InputPeripheralButtons &buttons) const
{
    const bool pressed[4] = {
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::LB),
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::RB),
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::LT),
        CoreSystems::GetInputHandler()->GetGamepadHold(buttons, WPadBtn::RT),
    };

    uint8 mask = pressed[0] << 0 | pressed[1] << 1 | pressed[2] << 2 | pressed[3] << 3;

    switch (mask)
    {
    case 0b0000: DrawButton(m_shoulderTexture, 1./16, 1, 0, 0); break;
    case 0b0001: DrawButton(m_shoulderTexture, 1./16, 1, 0, 1); break;
    case 0b0010: DrawButton(m_shoulderTexture, 1./16, 1, 0, 2); break;
    case 0b0100: DrawButton(m_shoulderTexture, 1./16, 1, 0, 3); break;
    case 0b1000: DrawButton(m_shoulderTexture, 1./16, 1, 0, 4); break;
    case 0b0011: DrawButton(m_shoulderTexture, 1./16, 1, 0, 5); break;
    case 0b0101: DrawButton(m_shoulderTexture, 1./16, 1, 0, 6); break;
    case 0b1001: DrawButton(m_shoulderTexture, 1./16, 1, 0, 7); break;
    case 0b0110: DrawButton(m_shoulderTexture, 1./16, 1, 0, 8); break;
    case 0b1010: DrawButton(m_shoulderTexture, 1./16, 1, 0, 9); break;
    case 0b1100: DrawButton(m_shoulderTexture, 1./16, 1, 0, 10); break;
    case 0b0111: DrawButton(m_shoulderTexture, 1./16, 1, 0, 11); break;
    case 0b1011: DrawButton(m_shoulderTexture, 1./16, 1, 0, 12); break;
    case 0b1101: DrawButton(m_shoulderTexture, 1./16, 1, 0, 13); break;
    case 0b1110: DrawButton(m_shoulderTexture, 1./16, 1, 0, 14); break;
    case 0b1111: DrawButton(m_shoulderTexture, 1./16, 1, 0, 15); break;
    default: break;
    }
}

void PeripheralWidget::RenderSequential(const uint8 seqState) const
{
    DrawButton(m_sequentiaTexture, 1./4, 1, 0, seqState);
}
void PeripheralWidget::RenderHShifter(const uint8 hState) const
{
    DrawButton(m_hShifterTexture, 1./8, 1, 0, hState + 1);
}

void PeripheralWidget::DrawButton(ImTextureID tex, float xUV, float yUV, int column, int row)
{
    const ImVec2 uv0(xUV * (float)row, yUV * (float)column);
    const ImVec2 uv1(xUV * ((float)row + 1.0f), yUV * ((float)column + 1.0f));
    ImGui::Image(tex, ImVec2(16, 16), uv0, uv1);
}
