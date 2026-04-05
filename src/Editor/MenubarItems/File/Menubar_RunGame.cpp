#include "Menubar_RunGame.h"

#include <Engine/Core/System/OS.h>

using namespace WEditor;

void Menubar_RunGame::Setup()
{
    m_entryName = "Run Game";
}

void Menubar_RunGame::OnClick()
{
    std::string exec = OS::GetProcessPath();
    wtl::vector<std::string> args {exec, "--game"};
    OS::CreateProcess(exec, args);
}
