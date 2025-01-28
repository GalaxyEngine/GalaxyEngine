#include "pch.h"
#include "Wrapper/PhysicAPI/CustomPhysics.h"

#include "Component/BoxCollider.h"
#include "Component/Transform.h"

#include "Component/RigidBody.h"
#include "Core/Application.h"

#include "Core/GameObject.h"
#include "Utils/Time.h"

namespace GALAXY 
{
	void Wrapper::PhysicAPI::CustomPhysicsAPI::Update()
	{
#ifdef WITH_EDITOR
		if (!Core::Application::IsPlayMode())
			return;
#endif
		float dt = Utils::Time::DeltaTime();

		for (auto& body : m_objectMap)
		{
			if (auto rigidbody = dynamic_cast<Component::RigidBody*>(body.first))
			{
				Component::Transform* transform = rigidbody->GetTransform();
				Vec3f position = transform->GetWorldPosition();
				Quat rotation = transform->GetWorldRotation();
				body.second.m_position = position;
				body.second.m_rotation = rotation;
			}
		}

		for (auto& body : m_objectMap)
		{
			body.second.m_velocity += body.second.m_gravityForce * dt;
			body.second.m_position += body.second.m_velocity * dt;
			body.second.m_rotation *= Quat::AngleAxis(body.second.m_omega.Length() * dt, body.second.m_omega);
		}

		for (auto& body : m_objectMap)
		{
			if (auto rigidbody = dynamic_cast<Component::RigidBody*>(body.first))
			{
				Component::Transform* transform = rigidbody->GetTransform();
				transform->SetWorldPosition(body.second.m_position);
				transform->SetWorldRotation(body.second.m_rotation);
			}
		}

		InternalUpdate();
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateRigidBody(Component::RigidBody* rigidbody)
	{
		Vec3f position = rigidbody->GetTransform()->GetWorldPosition();
		Quat rotation = rigidbody->GetTransform()->GetWorldRotation();
		InternalRigidbody body;
		body.m_position = position;
		body.m_rotation = rotation;
		body.m_gravityForce = defaultGravity;
		
		m_objectMap[rigidbody] = body;
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyRigidBody(Component::RigidBody* rigidbody)
	{
		auto object = m_objectMap.find(rigidbody); // Use auto, no reference
		if (object == m_objectMap.end())
		{
			PrintError("Could not find rigidbody associated with component 0x%x !", rigidbody);
			return;
		}
		m_objectMap.erase(object); // Erase using the iterator
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateBoxCollider(Component::BoxCollider* collider)
	{

	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyBoxCollider(Component::BoxCollider* collider)
	{

	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateSphereCollider(Component::SphereCollider* collider)
	{
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroySphereCollider(Component::SphereCollider* collider)
	{
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::SetDefaultGravity(const Vec3f& value)
	{
		defaultGravity = value;
	}

	bool Wrapper::PhysicAPI::CustomPhysicsAPI::InitializeAPI()
	{
		PrintLog("Custom Physics Initialized");
		return true;
	}

	Wrapper::PhysicAPI::CustomPhysicsAPI::~CustomPhysicsAPI()
	{

	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::InternalUpdate()
	{
		auto objects = BroadPhase();
	}

	struct SAPAABB
	{
		float Min;
		float Max;
	};

	std::list<std::tuple<Component::BaseComponent*>> Wrapper::PhysicAPI::CustomPhysicsAPI::BroadPhase()
	{
		auto colliders = m_colliderMap;
		std::vector<SAPAABB> aabb;
		for (auto& collider : colliders)
		{
			SAPAABB box;
			// auto AABB = collider.second.GetAABB();
			aabb.push_back(box);
		}
		std::list<std::tuple<Component::BaseComponent*>> result;
		return result;
	}
}
