#include "pch.h"
#include "Component/Transform.h"

#include "Resource/Scene.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#ifdef WITH_EDITOR
#include "Editor/ActionManager.h"
#include "Editor/UI/IconManager.h"
#endif
/*TODO :
 * Implement missing methods
 */
namespace GALAXY
{
	Component::Transform::Transform()
	{
	}

	Component::Transform::Transform(const Vec3f& position, const Quat& rotation, const Vec3f& scale /*= { 1, 1, 1 }*/)
	{

	}

	Vec3f Component::Transform::GetWorldPosition() const
	{
		if (p_gameObject && p_gameObject->GetParent())
		{
			return p_gameObject->GetParent()->GetTransform()->GetModelMatrix() * Vec4f(m_localPosition, 1.0f);
		}
		return m_localPosition;
	}

	Quat Component::Transform::GetWorldRotation() const
	{
		if (p_gameObject && p_gameObject->GetParent())
		{
			return p_gameObject->GetParent()->GetTransform()->GetWorldRotation() * m_localRotation;
		}
		return m_localRotation;
	}

	Vec3f Component::Transform::GetWorldScale() const
	{
		if (p_gameObject && p_gameObject->GetParent())
		{
			return GetModelMatrix().GetScale();
		}
		return m_localScale;
	}

	Vec3f Component::Transform::GetWorldEulerRotation() const
	{
		if (p_gameObject && p_gameObject->GetParent())
		{
			return GetModelMatrix().GetRotation().ToEuler();
		}
		return m_localEulerRotation;
	}

	void Component::Transform::OnUpdate()
	{
		m_wasDirty = false;
		if (!m_dirty)
			return;
		m_wasDirty = true;
		ForceUpdate();
	}

	void Component::Transform::ForceUpdate()
	{
		EOnUpdate.Invoke();
		if (p_gameObject && p_gameObject->GetParent())
			ComputeModelMatrix(p_gameObject->GetParent()->GetTransform()->GetModelMatrix());
		else
			ComputeModelMatrix();


		if (p_gameObject)
		{
			for (auto& child : p_gameObject->GetChildren())
			{
				child.lock()->GetTransform()->ForceUpdate();
			}
		}
	}


#ifdef WITH_EDITOR
	void Component::Transform::ShowInInspector()
	{
		Vec3f position = m_localPosition;
		Vec3f rotation = m_localEulerRotation;
		Vec3f scale = m_localScale;

		/* Position Vec3 control */
		{
			static Vec3f previousPosition;
			static bool previousTrue = false;
			if (Wrapper::UI::DrawVec3Control("Position", &position.x))
			{
				if (!previousTrue)
					previousPosition = m_localPosition;
				previousTrue = true;
			}
			else if (previousTrue && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				previousTrue = false;
				const Editor::Action action(
					[this, localPosition = position]()
					{
						SetLocalPosition(localPosition);
					},
					[this, localPosition = previousPosition]()
					{
						SetLocalPosition(localPosition);
					});

				Core::SceneHolder::GetCurrentScene()->GetActionManager()->AddAction(action);
			}
		}

		/* Rotation Vec3 control */
		{
			static Vec3f previousRotation;
			static bool previousTrue = false;
			if (Wrapper::UI::DrawVec3Control("Rotation", &rotation.x))
			{
				if (!previousTrue)
					previousRotation = m_localEulerRotation;
				previousTrue = true;
			}
			else if (previousTrue && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				previousTrue = false;
				const Editor::Action action(
					[this, localPosition = rotation]()
					{
						SetLocalRotation(localPosition);
					},
					[this, localPosition = previousRotation]()
					{
						SetLocalRotation(localPosition);
					});

				Core::SceneHolder::GetCurrentScene()->GetActionManager()->AddAction(action);
			}
		}

		/* Scale Vec3 control */
		{
			static Vec3f previousScale;
			static bool previousTrue = false;
			if (Wrapper::UI::DrawVec3Control("Scale", &scale.x, 1.f, true))
			{
				if (!previousTrue)
					previousScale = m_localScale;
				previousTrue = true;
			}
			else if (previousTrue && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				previousTrue = false;
				const Editor::Action action(
					[this, localPosition = scale]()
					{
						SetLocalScale(localPosition);
					},
					[this, localPosition = previousScale]()
					{
						SetLocalScale(localPosition);
					});

				Core::SceneHolder::GetCurrentScene()->GetActionManager()->AddAction(action);
			}
		}
		
		if (position != m_localPosition || rotation != m_localEulerRotation || scale != m_localScale) {
			SetLocalPosition(position);
			SetLocalRotation(rotation);
			SetLocalScale(scale);
		}

		const bool showDebug = false;
		if (!showDebug)
			return;
		ImGui::SeparatorText("Debug");
		
		Vec3f worldPosition = GetWorldPosition();
		Vec3f worldRotation = GetWorldRotation().ToEuler();
		Vec3f worldScale = GetWorldScale();

		if (ImGui::DragFloat3("World Position", &worldPosition.x))
		{
			SetWorldPosition(worldPosition);
		}
		if (ImGui::DragFloat3("World Rotation", &worldRotation.x))
		{
			SetWorldRotation(worldRotation);
		}
		if (ImGui::DragFloat3("World Scale", &worldScale.x))
		{
			SetWorldScale(worldScale);
		}
	}

	uint32_t Component::Transform::GetIcon()
	{
		return Editor::UI::IconManager::TransformComponentIcon;
	}
#endif

	void Component::Transform::Serialize(CppSer::Serializer& serializer)
	{
		serializer << CppSer::Pair::Key << "Position" << CppSer::Pair::Value << m_localPosition;
		serializer << CppSer::Pair::Key << "Rotation" << CppSer::Pair::Value << m_localRotation;
		serializer << CppSer::Pair::Key << "Scale" << CppSer::Pair::Value << m_localScale;
	}

	void Component::Transform::Deserialize(CppSer::Parser& parser)
	{
		SetLocalPosition(parser["Position"].As<Vec3f>());
		SetLocalRotation(parser["Rotation"].As<Quat>());
		SetLocalScale(parser["Scale"].As<Vec3f>());
	}
		
	void Component::Transform::SetWorldPosition(const Vec3f& worldPosition)
	{
	    if (p_gameObject && p_gameObject->GetParent())
	    {
	        // Get parent's model matrix and invert it.
	        auto* parentTransform = p_gameObject->GetParent()->GetTransform();
	        Mat4 parentModelMatrix = parentTransform->GetModelMatrix();
	        Mat4 parentInverse = parentModelMatrix.CreateInverseMatrix();
	        
	        // Transform the world position into the parent's local space.
	        Vec4f localPos4 = parentInverse * Vec4f(worldPosition, 1.0f);
	        Vec3f localPos(localPos4.x, localPos4.y, localPos4.z);
	        
	        SetLocalPosition(localPos);
	    }
	    else
	    {
	        SetLocalPosition(worldPosition);
	    }
	}

	void Component::Transform::SetWorldRotation(const Quat& worldRotation)
	{
	    if (p_gameObject && p_gameObject->GetParent())
	    {
	        auto* parentTransform = p_gameObject->GetParent()->GetTransform();
	        // Retrieve the parent's world rotation.
	        Quat parentWorldRotation = parentTransform->GetWorldRotation();
	        
	        // Compute the local rotation by "undoing" the parent's rotation.
	        Quat localRotation = parentWorldRotation.GetInverse() * worldRotation;
	        
	        SetLocalRotation(localRotation);
	    }
	    else
	    {
	        SetLocalRotation(worldRotation);
	    }
	}

	void Component::Transform::SetWorldRotation(const Vec3f& worldEulerRotation)
	{
	    // Convert the Euler angles to a quaternion.
	    // Adjust the conversion if your engine uses a different convention or rotation order.
	    Quat worldQuat = Quat::FromEuler(worldEulerRotation);
	    SetWorldRotation(worldQuat);
	}

	void Component::Transform::SetWorldScale(const Vec3f& worldScale)
	{
	    if (p_gameObject && p_gameObject->GetParent())
	    {
	        auto* parentTransform = p_gameObject->GetParent()->GetTransform();
	        Vec3f parentWorldScale = parentTransform->GetWorldScale();
	        
	        // Compute the local scale by dividing the world scale by the parent's world scale.
	        // Make sure to handle division-by-zero if that’s a possibility.
	        Vec3f localScale = Vec3f(
	            parentWorldScale.x != 0.0f ? worldScale.x / parentWorldScale.x : worldScale.x,
	            parentWorldScale.y != 0.0f ? worldScale.y / parentWorldScale.y : worldScale.y,
	            parentWorldScale.z != 0.0f ? worldScale.z / parentWorldScale.z : worldScale.z
	        );
	        
	        SetLocalScale(localScale);
	    }
	    else
	    {
	        SetLocalScale(worldScale);
	    }
	}
}