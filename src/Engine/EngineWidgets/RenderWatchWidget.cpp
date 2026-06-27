#include "RenderWatchWidget.h"

#include "Engine/EngineDefines.h"
#include "Engine/Core/Handlers/RenderHandler.h"
#include "Engine/Core/System/Iris.h"
#include "Engine/Types/CoreSystems.h"
#include <Engine/Types/Rendering/LightingInfo.h>

using namespace WEngine;

void RenderWatchWidget::Setup()
{
    m_widgetName = "Render Watch";
}

void RenderWatchWidget::RenderInternal()
{
    SetSize({300, 400});
    ImGui::Text("%s", GetHeader().c_str());
    ImGui::Separator();

    float32* sunDir;
    float32 sunCol[4];
    auto sunDirV = CoreSystems::GetRenderHandler()->GetSkylight();

    sunDir = (float32*)&sunDirV.direction;
    sunCol[0] = sunDirV.lightColor.red / 255.0f;
    sunCol[1] = sunDirV.lightColor.green / 255.0f;
    sunCol[2] = sunDirV.lightColor.blue / 255.0f;
    sunCol[3] = sunDirV.lightColor.alpha / 255.0f;

    ImGui::DragFloat3("Sun Direction", sunDir, 0.001f);
    ImGui::ColorEdit4("Sun Color", sunCol);

    sunDirV.lightColor.red = sunCol[0] * 255.0f;
    sunDirV.lightColor.green = sunCol[1] * 255.0f;
    sunDirV.lightColor.blue = sunCol[2] * 255.0f;
    sunDirV.lightColor.alpha = sunCol[3] * 255.0f;

    CoreSystems::GetRenderHandler()->SetSkylight(sunDirV);

    auto statInstBuf = Iris::GetStatInstBufAllocInfo();

    for (const auto& allocInfo : statInstBuf)
    {
        ImGui::PushID(&allocInfo);
        ImGui::Text("%s", StatInstInfoToString(allocInfo).c_str());
        ImGui::PopID();
    }
}

std::string RenderWatchWidget::GetHeader() const
{
    std::string header = "Rendering Info (";

    switch (GPU_BACKEND)
    {
        case GPU_OPENGL:    header += "OpenGL";     break;
        case GPU_VULKAN:    header += "Vulkan";     break;
        case GPU_D3D12:     header += "D3D12";      break;
        case GPU_METAL:     header += "Metal";      break;
        case GPU_AGC:       header += "GNM";        break;
        case GPU_NVN:       header += "NVN";        break;
        default:
            header += "Unknown Backend";
            break;
    }

    header += ")";
    return header;
}

std::string RenderWatchWidget::StatInstInfoToString(const MemListDebugInfo& info) const
{
    std::string text = "";
    if (info.model == 0 || info.material == 0)
        text += "FREE:\t";
    else
        text += "USED:\t";

    text += std::to_string(info.size) + 'B';
    return text;
}
