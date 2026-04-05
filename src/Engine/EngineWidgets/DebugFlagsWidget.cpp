#include "DebugFlagsWidget.h"

#include <sstream>

#include "Engine/Core/Handlers/WidgetHandler.h"
#include "Engine/Types/DebugFlags.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

void DebugFlagsWidget::Setup()
{
    m_widgetName = "Debug Flags";
}

void DebugFlagsWidget::RenderInternal()
{
    SetSize(Vector2(400, 450));
    ImGui::Text("Debug Flags");
    ImGui::Separator();

    if (ImGui::BeginTable("worldSelectTable", 17, ImGuiTableFlags_Borders))
    {
        for (uint16 i = 0; i < 17; i++)
        {
            if (i == 0)
            {
                ImGui::TableSetupColumn("");
                continue;
            }
            std::stringstream stream;
            stream << std::uppercase << std::hex << i - 1;
            ImGui::TableSetupColumn(std::format("{}", stream.str()).c_str());
        }
        ImGui::TableHeadersRow();
        for (uint16 i = 0; i < 16; i++)
        {
            ImGui::TableNextRow();

            for (uint16 j = 0; j < 17; j++)
            {
                ImGui::TableSetColumnIndex(j);
                ImGui::PushID(std::format("{}x{}", i, j).c_str());
                if (j == 0)
                {
                    std::stringstream stream;
                    stream << std::uppercase << std::hex << i;
                    ImGui::Text("%s", std::format("{}", stream.str()).c_str());
                    ImGui::PopID();
                    continue;
                }
                bool flag = DebugFlags::GetFlag(i, j - 1);
                ImGui::Checkbox("", &flag);
                DebugFlags::SetFlag(i, j - 1, flag);
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}
