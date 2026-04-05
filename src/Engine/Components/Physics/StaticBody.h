#pragma once

#include <Engine/Components/Component.h>

namespace WEngine
{
	class Entity;
	class PhysicsHandler;
	class SimulatableObject;
	class StaticBody : public Component
	{
	public:
		StaticBody(Entity* e);
		~StaticBody() override;

	private:
		SimulatableObject* m_simulatableObject;
		PhysicsHandler* m_physicsHandler;
	public:
		[[nodiscard]] SimulatableObject* GetSimulatableObject() const { return m_simulatableObject; }

	private:
		void Init();

		void Awake(ComponentArgs ca) override;
		void PhysicsTick(float32 tr) override;

		void UnloadComponent() override;

		COMP_HASH(0xf703e38529d5f51)
	};
}
