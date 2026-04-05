#pragma once
#include "Engine/Math/Vector.h"

namespace WEditor
{
    class Editor
    {
    public:
        Editor(int argc, char* argv[]);

    private:
        void StartEditor();
        void StartupMessage();

        void InitHandlers();

        [[noreturn]]
        void Run();

        void Dock();
    };
}

