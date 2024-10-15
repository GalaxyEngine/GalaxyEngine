#include "pch.h"
#include "Wrapper/PhysicAPI/CustomPhysics.h"

#include "Component/BoxCollider.h"
#include "Component/Transform.h"

#include "Component/Rigidbody.h"
#include "Core/Application.h"

#include "Core/GameObject.h"
#include "Utils/Time.h"

namespace GALAXY 
{
	void Wrapper::PhysicAPI::CustomPhysicsAPI::Update()
	{
		if (!Core::Application::IsPlayMode())
			return;
		float dt = Utils::Time::DeltaTime();

		for (auto& body : m_objectMap)
		{
			if (auto rigidbody = dynamic_cast<Component::Rigidbody*>(body.first))
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
		}

		for (auto& body : m_objectMap)
		{
			if (auto rigidbody = dynamic_cast<Component::Rigidbody*>(body.first))
			{
				Component::Transform* transform = rigidbody->GetTransform();
				transform->SetWorldPosition(body.second.m_position);
				transform->SetWorldRotation(body.second.m_rotation);
			}
		}
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateRigidbody(Component::Rigidbody* rigidbody)
	{
		Vec3f position = rigidbody->GetTransform()->GetWorldPosition();
		Quat rotation = rigidbody->GetTransform()->GetWorldRotation();
		InternalRigidBody body;
		body.m_position = position;
		body.m_rotation = rotation;
		body.m_gravityForce = defaultGravity;
		

		m_objectMap[rigidbody] = body;
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyRigidbody(Component::Rigidbody* rigidbody)
	{
		auto& object = m_objectMap.find(rigidbody);
		if (object == m_objectMap.end())
		{
			PrintError("Could not find rigidbody associated with component 0x%x !", rigidbody);
			return;
		}
		m_objectMap.erase(object);
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

	void GALAXY::Wrapper::PhysicAPI::CustomPhysicsAPI::InternalUpdate()
	{
	}
}
