#pragma once

#include <Engine/Math/Vector.h>
#include <Engine/Types/CommonTypes.h>
#include <Engine/WTL/vector.h>

#include <Engine/Util/Log.h>

namespace WEngine
{
    enum class PhysicsAreaType
    {
        None = 0,
        Circle,
        Rectangle,
        WorldBoundary,
        Cylinder,
        Polygon,
    };

    enum class PhysicsAreaCollisionType
    {
        Disabled,
        Collision,
        Trigger
    };

    enum class WorldBoundaryOrientation
    {
        HorizontalBottom,
        HorizontalTop,
        VerticalLeft,
        VerticalRight
    };

    class PhysicsArea
    {
    public:
        PhysicsArea() : m_offset(Vector2::Zero) {}
        virtual ~PhysicsArea() = default;

    public:
        void SetOffset(const Vector2 offset) { m_offset = offset; }
        void SetType(const PhysicsAreaCollisionType type) { m_type = type; }
        [[nodiscard]] Vector2 GetOffset() const { return m_offset; }
        [[nodiscard]] PhysicsAreaCollisionType GetType() const { return m_type; }

    private:
        Vector2 m_offset;
        PhysicsAreaCollisionType m_type;
    };

    // -------------------------------------------- [ Circle ] --------------------------------------------
    class PhysicsAreaCircle : public PhysicsArea
    {
    public:
        PhysicsAreaCircle() : m_radius(0.0f) {}
        PhysicsAreaCircle(const float32 radius) : m_radius(radius) {}

    public:
        void SetRadius(const float32 radius)
        {
            if (radius < 0.0f)
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Tried to set a negative radius!");
                return;
            }
            m_radius = radius;
        }
        [[nodiscard]] float32 GetRadius() const { return m_radius; }
    private:
        float32 m_radius;
    };

    // ------------------------------------------ [ Rectangle ] -------------------------------------------
    class PhysicsAreaRectangle: public PhysicsArea
    {
    public:
        PhysicsAreaRectangle() : m_size(Vector2::Zero) {}
        PhysicsAreaRectangle(const Vector2& size) : m_size(size) {}

    public:
        void SetBox(const Vector2 size)
        {
            if (size.x < 0 || size.y < 0)
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Tried to set a rect the size is negative!");
                return;
            }
            m_size = size;
        }
        [[nodiscard]] Vector2 GetBoxTopLeft() const { return { -(m_size.x / 2) , (m_size.y / 2) }; }
        [[nodiscard]] Vector2 GetBoxBottomRight() const { return { (m_size.x / 2) , -(m_size.y / 2) }; }
        [[nodiscard]] Vector2 GetBoxSize() const { return m_size; }
    private:
        Vector2 m_size;
    };

    // ---------------------------------------- [ World Boundary ] ----------------------------------------
    class PhysicsAreaWorldBoundary : public PhysicsArea
    {
    public:
        // After careful consultation with me, myself and I; we have decided to make horizontal bottom the default
        PhysicsAreaWorldBoundary() : m_orientation(WorldBoundaryOrientation::HorizontalBottom) {}
        PhysicsAreaWorldBoundary(const WorldBoundaryOrientation orientation) : m_orientation(orientation) {}

    public:
        void SetOrientation(const WorldBoundaryOrientation orientation) { m_orientation = orientation; }
        [[nodiscard]] WorldBoundaryOrientation GetOrientation() const { return m_orientation; }

    private:
        WorldBoundaryOrientation m_orientation;
    };

    // -------------------------------------------- [ Capsule ] -------------------------------------------
    class PhysicsAreaCapsule : public PhysicsArea
    {
    public:
        PhysicsAreaCapsule() : m_radius(0.0f), m_midHeight(0.0f) {}
        PhysicsAreaCapsule(const float32 radius, const float32 midHeight) : m_radius(radius), m_midHeight(midHeight) {}

    public:
        void SetRadius(const float32 radius)
        {
            if (radius < 0.0f)
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Tried to set a negative radius!");
                return;
            }
            m_radius = radius;
        }
        void SetMidHeight(const float32 midHeight)
        {
            if (midHeight < 0.0f)
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Tried to set a negative height!");
                return;
            }
            m_midHeight = midHeight;
        }
        [[nodiscard]] float32 GetRadius() const { return m_radius; }
        [[nodiscard]] float32 GetMidHeight() const { return m_midHeight; }
        [[nodiscard]] float32 GetTotalHeight() const { return m_midHeight + m_radius * 2; }


    private:
        float32 m_radius;
        float32 m_midHeight;
    };

    // -------------------------------------------- [ Polygon ] -------------------------------------------
    class PhysicsAreaPolygon : public PhysicsArea
    {
    public:
        PhysicsAreaPolygon() : m_vertices({Vector2::Zero, Vector2::Zero, Vector2::Zero}) {}
        PhysicsAreaPolygon(const wtl::vector<Vector2>& vertices) : m_vertices(vertices) {}

    public:
        void SetPolygons(const wtl::vector<Vector2>& vertices)
        {
            if (vertices.size() < 3)
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Tried to set a polygon with less than 3 vertices!");
                return;
            }
            m_vertices = vertices;
        }
        [[nodiscard]] const wtl::vector<Vector2>& GetPolygons() const { return m_vertices; }
    private:
        wtl::vector<Vector2> m_vertices;
    };
}

