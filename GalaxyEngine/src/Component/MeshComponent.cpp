#include "pch.h"
#include "Component/MeshComponent.h"
#include "Resource/Mesh.h"
#include "Core/GameObject.h"

namespace GALAXY {

	void Component::MeshComponent::OnDraw()
	{
		if (!m_mesh.lock())
			return;
		m_mesh.lock()->Render(gameObject.lock()->GetTransform()->GetModelMatrix());
	}

	void Component::MeshComponent::SetMesh(const std::weak_ptr<Resource::Mesh>& mesh)
	{
		if (!mesh.lock())
			return;
		m_mesh = mesh;
	}
}