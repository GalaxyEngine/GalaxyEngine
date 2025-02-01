#include "pch.h"
#include "Component/Collider.h"
#include "Component/RigidBody.h"

namespace GALAXY 
{
	Component::ColliderType Component::Collider::GetType()
	{
		return Count;
	}

	void Component::Collider::ShowInInspector()
	{
		ImGui::Checkbox("Draw AABB", &p_drawAABB);
	}

	void Component::Collider::OnEditorDraw()
	{
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		if (p_drawAABB)
		{
			Physic::AABB aabb = GetAABB();
			renderer->DrawWireCube(aabb.GetCenter(), aabb.GetExtents(), Vec4f(1, 1, 0, 1), 5.f);
		}
	}

	void Component::Collider::OnUpdate()
	{
		p_debugCollide = false;
	}

	Vec3f Component::Collider::Support(const Vec3f& direction)
	{
		return {};
	}
}
