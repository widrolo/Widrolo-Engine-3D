#include "Menubar_NodeTest.h"

#include "Editor/Core/Handlers/EditorUIHandler.h"
#include "Editor/EditorWidgets/Tests/NodeTestWidget.h"
#include "Editor/Types/EditorSystems.h"
#include "Engine/Core/System/Memory.h"

using namespace WEditor;

void Menubar_NodeTest::Setup()
{
    m_entryName = "Node Test";
}

void Menubar_NodeTest::OnClick()
{
    EditorSystems::GetEditorUIHandler()->AddEditorWidget(WAllocator::Construct<NodeTestWidget>(), true);
}
