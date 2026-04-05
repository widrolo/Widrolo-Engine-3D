#include <Engine/Core/Widget.h>
#include <Engine/Types/Input/InputDevice.h>
#pragma once

namespace WEngine
{

    class PeripheralWidget : public Widget
    {
    public:
        using Widget::Widget;
    private:
        ImTextureID m_faceButtonTexture;
        ImTextureID m_dPadTexture;
        ImTextureID m_shoulderTexture;

        ImTextureID m_sequentiaTexture;
        ImTextureID m_hShifterTexture;
    public:
        void Setup() override;
    protected:
        void RenderInternal() override;
    private:
        void RenderController(const InputPeripheral& p) const;
        void RenderSteeringWheel(const InputPeripheral& p) const;

        void RenderFaceButtons(const InputPeripheralButtons& buttons) const;
        void RenderDPad(const InputPeripheralButtons& buttons) const;
        void RenderShoulder(const InputPeripheralButtons& buttons) const;

        void RenderSequential(uint8 seqState) const;
        void RenderHShifter(uint8 hState) const;

        static void DrawButton(ImTextureID tex, float xUV, float yUV, int column, int row);
    };
} // WEngine