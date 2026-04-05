#include "Menubar_ResFind.h"

#include "Editor/Core/Handlers/EditorUIHandler.h"
#include "Editor/Types/EditorSystems.h"

#include <Editor/EditorWidgets/Tools/ResolutionFinderWidget.h>

using namespace WEditor;

void Menubar_ResFind::Setup()
{
    m_entryName = "Resolution Finder";
}

void Menubar_ResFind::OnClick()
{
    EditorSystems::GetEditorUIHandler()->AddEditorWidget(WAllocator::Construct<ResolutionFinderWidget>(), true);
}
