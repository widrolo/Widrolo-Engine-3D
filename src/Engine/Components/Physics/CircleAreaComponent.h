#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>

namespace WEngine
{
    class SimulatableObject;
    class CircleAreaComponent : public Component
    {
    public:
        CircleAreaComponent(Entity* e);
        ~CircleAreaComponent();

    private:
        SimulatableObject* m_simulatableObject;
        Vector2 m_offset;
        float32 m_radius;

    public:
        void TryAttach();

        void Awake(ComponentArgs ca) override;
        void Start() override;
        void Draw() override;

    private:
        void Init();

        COMP_HASH(0x7f2e413ae154da82);
    };
}

