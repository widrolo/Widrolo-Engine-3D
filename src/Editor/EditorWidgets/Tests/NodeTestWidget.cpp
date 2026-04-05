#include "NodeTestWidget.h"

using namespace WEditor;

void NodeTestWidget::Setup()
{
    m_widgetName = "Node Test Widget";
}

void NodeTestWidget::RenderInternal()
{
    SetSize(WEngine::Vector2(1000, 800));
    ImGui::Text("Node Test Widget");
    ImGui::Separator();



}
