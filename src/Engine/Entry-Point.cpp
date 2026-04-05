#define IMGUI_DEFINE_MATH_OPERATORS
#include <Engine/imgui/imgui.h>

#include <Engine/Core/Engine.h>
#include <Editor/Core/Editor.h>
#include <Engine/Core/System/Memory.h>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

#ifdef DEBUG // i should never accidentally ship only the editor...
#define Force_Editor false
#else
#define Force_Editor false
#endif

#ifdef DEBUG
int main(int argc, char* argv[])
{
    // Yes, engine is in heap, kill me!
    WAllocator::BootAllocator();

    bool launchEditor = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--wedge")
            launchEditor = true;

        // this is so the editor can launch the game even when compiled with Force_Editor on
        if (std::string(argv[i]) == "--game")
        {
            auto* engine = WAllocator::Construct<WEngine::Engine>(argc, argv);
            return 0;
        }
    }

    if (launchEditor || Force_Editor)
        auto* editor = WAllocator::Construct<WEditor::Editor>(argc, argv);
    else
        auto* engine = WAllocator::Construct<WEngine::Engine>(argc, argv);

    return 0;
}
#endif


#ifdef RC
int main(int argc, char* argv[])
{
    WAllocator::BootAllocator();
    bool launchEditor = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--wedge")
            launchEditor = true;
    }

    if (launchEditor || Force_Editor)
        auto* editor = WAllocator::Construct<WEditor::Editor>(argc, argv);
    else
        auto* engine = WAllocator::Construct<WEngine::Engine>(argc, argv);
    return 0;
}
#endif

#ifdef PACKAGE

#ifdef WE_Windows
#include <Windows.h>

extern int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    WAllocator::BootAllocator();
    // Parse cmdline into argc and argv
    std::string cmdStr(cmdline);
    std::istringstream iss(cmdStr);
    std::vector<std::string> args;
    std::string arg;

    while (iss >> std::quoted(arg)) {
        args.push_back(arg);
    }

    int argc = static_cast<int>(args.size());
    std::vector<char*> argv;
    for (auto& s : args) {
        argv.push_back(&s[0]); // get pointer to mutable string buffer
    }

    for (auto a : argv)
    {
        std::cout << a << std::endl;
    }



    bool launchEditor = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--wedge")
            launchEditor = true;
    }

    if (launchEditor || Force_Editor)
        auto* editor = WAllocator::Construct<WEditor::Editor>(argc, argv.data());
    else
        auto* engine = WAllocator::Construct<WEngine::Engine>(argc, argv.data());
    return 0;
}

#else

int main(int argc, char* argv[])
{
    WAllocator::BootAllocator();
    bool launchEditor = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--wedge")
            launchEditor = true;
    }

    if (launchEditor || Force_Editor)
        auto* editor = WAllocator::Construct<WEditor::Editor>(argc, argv);
    else
        auto* engine = WAllocator::Construct<WEngine::Engine>(argc, argv);
    return 0;
}

#endif
#endif
