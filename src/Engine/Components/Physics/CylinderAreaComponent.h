#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>

namespace WEngine
{
    class SimulatableObject;
    class CylinderAreaComponent : public Component
    {
    public:
        CylinderAreaComponent(Entity* e);
        ~CylinderAreaComponent();

    private:
        SimulatableObject* m_simulatableObject;
        Vector2 m_offset;
        float32 m_radius;
        float32 m_midHeight;

    public:
        void Awake(ComponentArgs ca) override;
        void Start() override;

        COMP_HASH(0xecf732458626839d);
    };
}
