#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>

namespace WEngine
{
    class SimulatableObject;
    class RectAreaComponent : public Component
    {
    public:
        RectAreaComponent(Entity* e);
        ~RectAreaComponent();

    private:
        SimulatableObject* m_simulatableObject;
        Vector2 m_offset;
        Vector2 m_areaSize;

    public:
        void TryAttach();
        void SetSize(Vector2 size);

        void Awake(ComponentArgs ca) override;
        void Start() override;
        void Draw() override;

    private:
        void Init();
        COMP_HASH(0xaf52ded390e23822);
    };
}