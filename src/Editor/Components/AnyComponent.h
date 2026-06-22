#pragma once

#include <variant>
#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>
#include <Engine/Types/Rendering/Color.h>

#include "Engine/Core/Physics/SimulatableObject.h"
#include "Engine/Types/Nullable.h"


namespace WEditor
{
    using OptionData = std::variant<int32, float32, bool, std::string, WEngine::Vector2, WEngine::Vector3, WEngine::Color>;

    class AnyComponent : public WEngine::Component
    {
    public:
        AnyComponent(WEngine::Entity* e);

    public:
        uint16 m_ID;
        std::vector<OptionData> m_data;
    public:

        void Init(uint16 ID, uint8 dataSize, WEngine::ComponentArgs args);
        void Init(uint16 ID, uint8 dataSize);
        void SetData(uint8 location, OptionData data);

        void ApplySpawnData(WEngine::ComponentArgs args);

        OptionData GetData(uint8 location);
        WEngine::Nullable<OptionData> FindDataByName(const std::string& name);

        void Draw() override;

    private:
        bool m_isInitializedDraw = false;
        WEngine::SimulatableObject simulatableObject;

        void TryDrawGameGraphics();
        void TryDrawDebugGraphics();
        void DrawOnSelected();

        void GFX_Dbg_CircleArea();
        void GFX_Dbg_RectArea();

        COMP_HASH(0xf22524fc1c003f87)
    };

}
