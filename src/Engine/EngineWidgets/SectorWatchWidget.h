#pragma once

#include <Engine/Core/Widget.h>

namespace WEngine
{
    class Sector;
    class SectorWatchWidget : public Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
        void ForceHideSector(Sector& sector);
        void ForceShowSector(Sector& sector);
        void ForceUnloadSector(Sector& sector);
    };
}