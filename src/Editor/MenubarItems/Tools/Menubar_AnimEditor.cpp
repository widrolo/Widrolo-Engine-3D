#include "Menubar_AnimEditor.h"

#include "Editor/Core/Handlers/EditorUIHandler.h"
#include "Editor/EditorWidgets/Tools/AnimationEditorWidget.h"
#include "Editor/Types/EditorSystems.h"

using namespace WEditor;

void Menubar_AnimEditor::Setup()
{
    m_entryName = "Animation Editor";
}

void Menubar_AnimEditor::OnClick()
{
    EditorSystems::GetEditorUIHandler()->AddEditorWidget(WAllocator::Construct<AnimationEditorWidget>(), true);
}
