#include "pch.h"
#include "Component/Collider.h"
#include "Component/RigidBody.h"
#include "Core/Application.h"

namespace GALAXY 
{
	Component::ColliderType Component::Collider::GetType() const
	{
		return Count;
	}

#ifdef WITH_EDITOR
	void Component::Collider::ShowInInspector()
	{
		ImGui::InputFloat("Restitution", &p_restitution, 0.1f, 1.f);
		if (Core::Application::IsPlayMode() || Core::Application::IsPauseMode())
		{
			ImGui::TextColored(p_debugCollide ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), p_debugCollide ? "Collision Detected" : "No Collision Detected");
		}
		ImGui::Checkbox("Draw AABB", &p_drawAABB);

		ImGui::InputFloat3("Support Direction", &p_supportDirection.x);
		if (ImGui::Button("Debug Support"))
		{
			p_supportPosition = Support(p_supportDirection);
		}
	}

	void Component::Collider::OnEditorDraw()
	{
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		if (p_drawAABB)
		{
			Physic::AABB aabb = GetAABB();
			renderer->DrawWireCube(aabb.GetCenter(), aabb.GetExtents(), Vec4f(1, 1, 0, 1), 5.f);
		}
		if (p_supportPosition.has_value())
			renderer->DrawWireCube(p_supportPosition.value(), Vec3f::One() * 0.1f, Vec4f(0, 0, 1, 1), 10.f);
	}
#endif

	void Component::Collider::OnUpdate()
	{
		p_debugCollide = false;
	}

	Vec3f Component::Collider::Support(const Vec3f& direction)
	{
		return {};
	}
}
