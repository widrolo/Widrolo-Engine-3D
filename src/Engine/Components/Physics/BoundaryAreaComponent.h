#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>

#include <Engine/Core/Physics/PhysicsArea.h>

namespace WEngine
{
    class SimulatableObject;
    class BoundaryAreaComponent : public Component
    {
    public:
        BoundaryAreaComponent(Entity* e);
        ~BoundaryAreaComponent();

    private:
        SimulatableObject* m_simulatableObject;
        Vector2 m_offset;
        WorldBoundaryOrientation m_orientation;

    public:
        void Awake(ComponentArgs ca) override;
        void Start() override;

        COMP_HASH(0xc0a43a4bc6956b3e);
    };
}
