#include "Menubar_Exit.h"

using namespace WEditor;

void Menubar_Exit::Setup()
{
    m_entryName = "Exit";
}

void Menubar_Exit::OnClick()
{
    std::exit(EXIT_SUCCESS);
}

