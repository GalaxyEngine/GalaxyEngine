#include "pch.h"
#include "Component/Collider.h"
#include "Component/Rigidbody.h"

namespace GALAXY 
{
	Component::ColliderType Component::Collider::GetType()
	{
		return Count;
	}
}
