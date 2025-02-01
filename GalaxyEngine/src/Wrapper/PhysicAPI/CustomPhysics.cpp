#include "pch.h"
#include "Wrapper/PhysicAPI/CustomPhysics.h"

#include "Component/BoxCollider.h"
#include "Component/SphereCollider.h"
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

		/*
		for (auto& body : m_objectMap)
		{
			if (auto rigidbody = dynamic_cast<Component::RigidBody*>(body.first))
			{
				Component::Transform* transform = rigidbody->GetTransform();
				Vec3f position = transform->GetWorldPosition();
				Quat rotation = transform->GetWorldRotation();
				// body.second.m_position = position;
				// body.second.m_rotation = rotation;
			}
		}
		*/

		for (const Weak<Component::RigidBody>& _body : m_objectMap)
		{
			Shared<Component::RigidBody> body = _body.lock();
			Component::Transform* transform = body->GetTransform();
			
			Vec3f newVelocity = body->GetVelocity() + body->GetGravityForce() * dt;
			Vec3f newPosition = transform->GetWorldPosition() + newVelocity * dt;
			Quat newRotation = transform->GetWorldRotation() * Quat::AngleAxis(body->GetAngularVelocity().Length() * dt, body->GetAngularVelocity());
			body->SetVelocity(newVelocity);
			
			transform->SetWorldPosition(newPosition);
			transform->SetWorldRotation(newRotation);
		}
		/*

		for (auto& body : m_objectMap)
		{
			if (auto rigidbody = dynamic_cast<Component::RigidBody*>(body.first))
			{
				Component::Transform* transform = rigidbody->GetTransform();
				transform->SetWorldPosition(body.second.m_position);
				transform->SetWorldRotation(body.second.m_rotation);
			}
		}
		*/
		InternalUpdate();
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateRigidBody(Weak<Component::RigidBody> rigidbody)
	{
		m_objectMap.insert(rigidbody);
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyRigidBody(Weak<Component::RigidBody> rigidbody)
	{
		auto object = m_objectMap.find(rigidbody); // Use auto, no reference
		if (object == m_objectMap.end())
		{
			PrintError("Could not find rigidbody associated with component 0x%x !", rigidbody);
			return;
		}
		m_objectMap.erase(object); // Erase using the iterator
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateBoxCollider(Weak<Component::BoxCollider> collider)
	{
		m_colliderMap.insert(collider);
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyBoxCollider(Weak<Component::BoxCollider> collider)
	{
		auto object = m_colliderMap.find(collider); // Use auto, no reference
		if (object == m_colliderMap.end())
		{
			PrintError("Could not find collider associated with component 0x%x !", collider);
			return;
		}
		m_colliderMap.erase(object); // Erase using the iterator
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateSphereCollider(Weak<Component::SphereCollider> collider)
	{
		m_colliderMap.insert(collider);
	}

	void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroySphereCollider(Weak<Component::SphereCollider> collider)
	{
		m_colliderMap.erase(collider);
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
		std::vector<ColliderPair> objects = BroadPhase();

		for (auto& pair : objects)
		{
			if (GJK(pair.first, pair.second))
			{
				pair.first->SetDebugCollide(true);
				pair.second->SetDebugCollide(true);
				// PrintLog("%s collide with %s",
					// pair.first->GetGameObject()->GetName().c_str(),
					// pair.second->GetGameObject()->GetName().c_str());
			}
		}
		
	}

	struct SAPAABB
	{
		Component::Collider* collider;
		Vec3f Min;
		Vec3f Max;
	};

	std::vector<Wrapper::PhysicAPI::ColliderPair> Wrapper::PhysicAPI::CustomPhysicsAPI::BroadPhase()
	{
		auto colliders = m_colliderMap;
		std::vector<SAPAABB> aabbs;
    
		// First, gather all AABBs from the colliders
		for (auto& _collider : colliders)
		{
			Shared<Component::Collider> collider = _collider.lock();
			if (collider)
			{
				SAPAABB box;
				Physic::AABB aabb = collider->GetAABB();
            
				// Assuming SAPAABB has min and max fields for the AABB bounds
				box.Min = aabb.Min;
				box.Max = aabb.Max;
				box.collider = collider.get(); // Store the collider pointer for later reference
            
				aabbs.push_back(box);
			}
		}
    
		// Sort AABBs along one axis (e.g., x-axis)
		std::ranges::sort(aabbs, [](const SAPAABB& a, const SAPAABB& b) {
			return a.Min.x < b.Min.x;
		});
    
		// Perform sweep and prune to find potential collisions
		std::vector<ColliderPair> result;
		for (size_t i = 0; i < aabbs.size(); ++i)
		{
			for (size_t j = i + 1; j < aabbs.size(); ++j)
			{
				// Check if the AABBs overlap
				if (aabbs[i].Max.x < aabbs[j].Min.x)
				{
					// No overlap, and since the list is sorted, we can break out of the inner loop
					break;
				}
            
				if (aabbs[i].Max.y >= aabbs[j].Min.y && aabbs[i].Min.y <= aabbs[j].Max.y &&
					aabbs[i].Max.z >= aabbs[j].Min.z && aabbs[i].Min.z <= aabbs[j].Max.z)
				{
					// AABBs overlap, add the colliders to the result
					result.emplace_back(aabbs[i].collider, aabbs[j].collider);
				}
			}
		}
    
		return result;
	}

	Vec3f Support(Component::Collider* a, Component::Collider* b, const Vec3f& direction) {
    return a->Support(direction) - b->Support(-direction);
}

bool SimplexContainsOrigin(std::vector<Vec3f>& simplex, Vec3f& direction) {
    const Vec3f a = simplex.back();
    const Vec3f ao = -a;

    if (simplex.size() == 3) { // Triangle case
        const Vec3f b = simplex[2];
        const Vec3f c = simplex[1];
        
        const Vec3f ab = b - a;
        const Vec3f ac = c - a;
        
        const Vec3f abPerp = ab.Cross(ac).Cross(ab);
        if (abPerp.Dot(ao) > 0) {
            simplex.erase(simplex.begin());
            direction = abPerp.GetNormalize();
            return false;
        }

        const Vec3f acPerp = ac.Cross(ab).Cross(ac);
        if (acPerp.Dot(ao) > 0) {
            simplex.erase(simplex.begin() + 1);
            direction = acPerp.GetNormalize();
            return false;
        }

        direction = ab.Cross(ac).GetNormalize();
        if (direction.Dot(ao) < 0) {
            direction = -direction;
        }
        return false;
    }
    else if (simplex.size() == 4) { // Tetrahedron case
        const Vec3f b = simplex[2];
        const Vec3f c = simplex[1];
        const Vec3f d = simplex[0];
        
        const Vec3f ab = b - a;
        const Vec3f ac = c - a;
        const Vec3f ad = d - a;
        
        const Vec3f abc = ab.Cross(ac);
        if (abc.Dot(ao) > 0) { // Fixed: Use ao instead of a
            simplex = {a, b, c};
            direction = abc.GetNormalize();
            return false;
        }

        const Vec3f acd = ac.Cross(ad);
        if (acd.Dot(ao) > 0) { // Fixed: Use ao instead of a
            simplex = {a, c, d};
            direction = acd.GetNormalize();
            return false;
        }

        const Vec3f adb = ad.Cross(ab);
        if (adb.Dot(ao) > 0) { // Fixed: Use ao instead of a
            simplex = {a, d, b};
            direction = adb.GetNormalize();
            return false;
        }

        return true; // Origin contained in tetrahedron
    }

    // Line segment case
    const Vec3f b = simplex[0];
    const Vec3f ab = b - a;
    
    if (ab.Dot(ao) > 0) {
        direction = ab.Cross(ao).Cross(ab).GetNormalize();
    } else {
        simplex = {a};
        direction = ao.GetNormalize();
    }
    return false;
}

bool Wrapper::PhysicAPI::CustomPhysicsAPI::GJK(Component::Collider* a, Component::Collider* b)
{
    Vec3f direction = b->GetTransform()->GetWorldPosition() - b->GetTransform()->GetWorldPosition(); // Better initial direction
    if (direction.LengthSquared() == 0)
        direction = Vec3f(1, 1, 1); // Fallback direction

    std::vector<Vec3f> simplex;
    simplex.push_back(Support(a, b, direction));
    direction = -simplex[0];

    const int maxIterations = 32; // Maximum number of iterations
    const float epsilon = 1e-6f; // Small epsilon for floating-point precision

    for (int i = 0; i < maxIterations; ++i)
    {
        Vec3f newSupport = Support(a, b, direction);

        if (newSupport.Dot(direction) < -epsilon)
            return false;

        simplex.push_back(newSupport);
        if (SimplexContainsOrigin(simplex, direction))
        {
            return true;
        }
    }
    return false;
}
}
