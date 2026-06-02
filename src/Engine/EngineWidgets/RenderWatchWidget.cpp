#include "RenderWatchWidget.h"

#include "Engine/EngineDefines.h"
#include "Engine/Core/System/Iris.h"

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
        case GPU_GNM:       header += "GNM";        break;
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
    if (info.model == 0 || info.shader == 0)
        text += "FREE:\t";
    else
        text += "USED:\t";

    text += std::to_string(info.size) + 'B';
    return text;
}
