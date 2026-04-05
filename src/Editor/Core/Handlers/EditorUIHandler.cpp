#include "EditorUIHandler.h"

#include <algorithm>
#include <Engine/Core/Widget.h>

#include <Editor/EditorWidgets/AboutWidget.h>
#include <Editor/EditorWidgets/ComponentList.h>
#include <Editor/EditorWidgets/ComponentSettings.h>
#include <Editor/EditorWidgets/EntityList.h>
#include <Editor/EditorWidgets/SectorList.h>
#include <Editor/EditorWidgets/Viewport.h>

#include "Engine/Util/Log.h"

using namespace WEditor;

static uint32 InitBaseWidget_count = 0;
template<class T = WEngine::Widget>
void InitBaseWidget(wtl::vector<WEngine::Widget*>& vec)
{
    vec.push_back(WAllocator::Construct<T>());
    vec[InitBaseWidget_count]->SetOpenState(true);
    vec[InitBaseWidget_count]->Setup();
    InitBaseWidget_count++;
}

EditorUIHandler::EditorUIHandler()
{
    InitSystemWidgets();
}

EditorUIHandler::~EditorUIHandler()
{

}

void EditorUIHandler::InitSystemWidgets()
{
    InitBaseWidget<ComponentList>(m_uiWidgets);
    InitBaseWidget<ComponentSettings>(m_uiWidgets);
    InitBaseWidget<EntityList>(m_uiWidgets);
    InitBaseWidget<Viewport>(m_uiWidgets);
    InitBaseWidget<SectorList>(m_uiWidgets);
}

void EditorUIHandler::DrawWidgets()
{
    for (const auto& widget : m_uiWidgets)
        widget->RenderWidget();
}

void EditorUIHandler::AddEditorWidget(WEngine::Widget *widget, bool openOnAdd)
{
    widget->Setup();
    bool foundSameWidget = false;
    WEngine::Widget* found;
    for (const auto& w : m_uiWidgets)
    {
        if (w->m_widgetName == widget->m_widgetName) // its just the editor, who cares about performance here?
        {
            foundSameWidget = true;
            found = w;
            break;
        }
    }

    if (!foundSameWidget)
    {
        m_uiWidgets.push_back(widget);
        widget->SetOpenState(openOnAdd);
    }
    else
    {
        WEngine::WLog::ConsoleLog("Already exists...");
        WAllocator::Destruct(widget);
        found->SetOpenState(openOnAdd);
    }
}

void EditorUIHandler::RemoveEditorWidget(const WEngine::Widget *widget)
{
    m_uiWidgets.erase(std::find(m_uiWidgets.begin(), m_uiWidgets.end(), widget));
}

