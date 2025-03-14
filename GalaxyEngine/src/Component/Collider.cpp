#include "pch.h"
#include "Component/Collider.h"
#include "Component/RigidBody.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

#include "Physic/CollisionLayer.h"

namespace GALAXY 
{
	Component::ColliderType Component::Collider::GetType() const
	{
		return Count;
	}

#ifdef WITH_EDITOR
	void Component::Collider::ShowInInspector()
	{
		auto layers = Physic::CollisionLayerManager::GetLayerNames();
		if (layers.empty())
		{
			ImGui::TextColored({1, 0, 0, 0}, "Error : No layers");
			return;
		}
		if (ImGui::BeginCombo("Layer", layers[p_collisionLayer].c_str()))
		{
			for (uint32_t i = 0; i < layers.size(); i++)
			{
				bool isSelected = p_collisionLayer == i;
				if (ImGui::Selectable(layers[i].c_str(), isSelected))
				{
					p_collisionLayer = i;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::InputFloat("Restitution", &p_restitution, 0.1f, 1.f);
		ImGui::InputFloat("Friction", &p_friction, 0.1f, 1.f);
		if (Core::Application::IsPlayMode() || Core::Application::IsPauseMode())
		{
			ImGui::TextColored(p_debugCollide ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), p_debugCollide ? "Collision Detected" : "No Collision Detected");
		}
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
#endif

	void Component::Collider::Serialize(CppSer::Serializer& serializer)
	{
		serializer << CppSer::Pair::Key << "CollisionLayer" << CppSer::Pair::Value << p_collisionLayer;
		serializer << CppSer::Pair::Key << "Restitution" << CppSer::Pair::Value << p_restitution;
		serializer << CppSer::Pair::Key << "Friction" << CppSer::Pair::Value << p_friction;
	}

	void Component::Collider::Deserialize(CppSer::Parser& parser)
	{
		p_collisionLayer = parser["CollisionLayer"].As<int>();
		p_restitution = parser["Restitution"].As<float>();
		p_friction = parser["Friction"].As<float>();
	}

	void Component::Collider::OnUpdate()
	{
		p_debugCollide = false;
	}

	Vec3f Component::Collider::Support(const Vec3f& direction)
	{
		return {};
	}

	void Component::Collider::InternalUpdateRigidbody()
	{
		if (!p_attachedRigidbody.expired())
			return;

		p_attachedRigidbody = GetGameObject()->GetComponent<RigidBody>();
	}
}
