#pragma once

#include <Engine/Core/Widget.h>
#include <array>
#include <Engine/Types/CommonTypes.h>

namespace WEditor
{
    struct ResolutionFinderEntry
    {
        uint16 width;
        uint16 height;
        bool selected;
    };
    class ResolutionFinderWidget : public WEngine::Widget
    {
    public:
        using WEngine::Widget::Widget;
    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    private:
        void FindNextResolution();

    private:
        // all usual resolutions, if true by default, then its a common
        // resolution.
        std::array<ResolutionFinderEntry, 14> m_resolutions = {
            ResolutionFinderEntry{256, 144, false},
            ResolutionFinderEntry{426, 240, false},
            ResolutionFinderEntry{640, 360, false},
            ResolutionFinderEntry{854, 480, false},
            ResolutionFinderEntry{960, 540, false},
            ResolutionFinderEntry{1024, 576, false},
            ResolutionFinderEntry{1280, 720, true},
            ResolutionFinderEntry{1366, 768, false},
            ResolutionFinderEntry{1600, 900, true},
            ResolutionFinderEntry{1920, 1080, true},
            ResolutionFinderEntry{2048, 1152, false},
            ResolutionFinderEntry{2560, 1440, true},
            ResolutionFinderEntry{3200, 1800, false},
            ResolutionFinderEntry{3840, 2160, true}
        };
    };
}

