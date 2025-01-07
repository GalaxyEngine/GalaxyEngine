#include "pch.h"
#include "Component/Collider.h"
#include "Component/RigidBody.h"

namespace GALAXY 
{
	Component::ColliderType Component::Collider::GetType()
	{
		return Count;
	}
}
