
#include "Menubar_About.h"

#include "Editor/Core/Handlers/EditorUIHandler.h"
#include "Editor/Types/EditorSystems.h"

#include <Editor/EditorWidgets/AboutWidget.h>

using namespace WEditor;

void Menubar_About::Setup()
{
    m_entryName = "About";
}

void Menubar_About::OnClick()
{
    EditorSystems::GetEditorUIHandler()->AddEditorWidget(WAllocator::Construct<AboutWidget>(), true);
}
