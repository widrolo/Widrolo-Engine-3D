#pragma once
#include "Engine/Core/Widget.h"

struct MemListDebugInfo;

namespace WEngine
{
    class Sector;
    class RenderWatchWidget : public Widget
    {
    public:
        using Widget::Widget;

    public:
        void Setup() override;
    protected:
        void RenderInternal() override;

    private:
        std::string GetHeader() const;
        std::string StatInstInfoToString(const MemListDebugInfo& info) const;
    };
}
