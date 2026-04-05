#pragma once

#include <Engine/Math/Vector.h>
#include <Engine/Types/CommonTypes.h>
#include <Engine/Core/Physics/PhysicsArea.h>
#include <Engine/Types/Nullable.h>

#include "Engine/Types/Physics/OverlapResult.h"

namespace WEngine
{
    class PhysicsSolver;
    class Entity;
    class SimulatableObject
    {
        friend PhysicsSolver;
    public:
        SimulatableObject();
        virtual ~SimulatableObject();

    public:
        [[nodiscard]] uint64 GetID() const { return m_id; }

        void SetOwner(Entity* entity) { m_owner = entity; }
        void SetPosition(const Vector2& position) { m_position = position; }
        void SetVelocity(const Vector2& velocity) { m_velocity = velocity; }
        void SetAcceleration(const Vector2& acceleration) { m_acceleration = acceleration; }
        void SetGravity(const float32 gravity) { m_gravity = gravity; }
        void SetDamping(const float32 damping) { m_damping = damping; }
        void SetFreezeState(const bool isLocked) { m_frozen = isLocked; }

        [[nodiscard]] Entity* GetOwner() const { return m_owner; }
        [[nodiscard]] Vector2 GetPosition() const { return m_position; }
        [[nodiscard]] Vector2 GetVelocity() const { return m_velocity; }
        [[nodiscard]] Vector2 GetAcceleration() const { return m_acceleration; }
        [[nodiscard]] float32 GetGravity() const { return m_gravity; }
        [[nodiscard]] float32 GetDamping() const { return m_damping; }
        [[nodiscard]] bool GetFreezeState() const { return m_frozen; }

        [[nodiscard]] PhysicsAreaType GetAreaType() const { return m_areaType; }
        void LockAreaType() { m_lockAreaType = true; }
        void UnlockAreaType() { m_lockAreaType = false; }
        void RemoveArea();

        void SetAreaOffset(const Vector2 offset) const { m_physicsArea->SetOffset(offset); }
        void SetAreaCircle(float32 radius);
        void SetAreaRect(Vector2 size);
        void SetAreaWorldBoundary(WorldBoundaryOrientation orientation);
        void SetAreaCapsule(float32 radius, float32 midHeight);
        void SetAreaPolygon(const wtl::vector<Vector2>& vertices);

        [[nodiscard]] Vector2 GetOffset() const { return m_physicsArea->GetOffset(); }
        [[nodiscard]] Nullable<PhysicsAreaCircle*> GetAreaCircle() const;
        [[nodiscard]] Nullable<PhysicsAreaRectangle*> GetAreaRect() const;
        [[nodiscard]] Nullable<PhysicsAreaWorldBoundary*> GetAreaWorldBoundary() const;
        [[nodiscard]] Nullable<PhysicsAreaCapsule*> GetAreaCapsule() const;
        [[nodiscard]] Nullable<PhysicsAreaPolygon*> GetAreaPolygon() const;

    private:
        static uint64 GetNextID();

    private:
        const uint64 m_id;

        Vector2 m_position;
        Vector2 m_velocity;
        Vector2 m_acceleration;

        float32 m_gravity;
        float32 m_damping;
        bool m_frozen;

        PhysicsAreaType m_areaType;
        PhysicsArea* m_physicsArea; // This shall always be cast to the proper m_areaType!
        bool m_lockAreaType; // This prevents the programm from accidentally switching area type

        Entity* m_owner;

    };
}

