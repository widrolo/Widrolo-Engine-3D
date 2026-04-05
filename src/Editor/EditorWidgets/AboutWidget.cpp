#include "AboutWidget.h"

#include <Engine/Util/Log.h>
using namespace WEditor;

void AboutWidget::Setup()
{
    m_widgetName = "About";
    m_windowFlags |= ImGuiWindowFlags_NoResize;
}

void AboutWidget::RenderInternal()
{
    SetSize({400, 350});

    ImGui::Text("[W]idrolo [E]nhanced [D]evelopment [G]ame [E]ditor");
}
