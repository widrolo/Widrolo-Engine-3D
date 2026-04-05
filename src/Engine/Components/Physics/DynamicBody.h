#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>

namespace WEngine
{
	class Entity;
	class PhysicsHandler;
	class SimulatableObject;
	class DynamicBody : public Component
	{
	public:
		DynamicBody(Entity* e);
		~DynamicBody() override;

	private:
		PhysicsHandler* m_physicsHandler;
		SimulatableObject* m_simulatableObject;

	public:
		void UpdateEntity() const;

		void SetPosition(Vector2 position) const;
		void SetVelocity(Vector2 velocity) const;
		void SetAcceleration(Vector2 acceleration) const;

		void SetGravity(float32 gravity) const;
		void SetDamping(float32 damping) const;
		// NOOOOO JESSIE, DONT ABBREVIATE START ACCELERATION!!!!
		void SetStartAcceleration(Vector2 sa) const;

		void FreezeObject() const;
		void UnfreezeObject() const;

		[[nodiscard]] SimulatableObject* GetSimulatableObject() const { return m_simulatableObject; }

	private:
		void Init();

		void Awake(ComponentArgs ca) override;
		void PhysicsTick(float32 tr) override;

		void UnloadComponent() override;

		COMP_HASH(0xfcf40b9c821cf519)
	};
}

