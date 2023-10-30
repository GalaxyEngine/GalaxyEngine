#pragma once
#include "Component/MeshComponent.h"
namespace GALAXY 
{
	inline void Component::MeshComponent::SetMesh(const Weak<Resource::Mesh>& mesh)
	{
		if (!mesh.lock())
			return;
		m_mesh = mesh;
	}
}
