#include "pch.h"
#include "Component/Transform.h"

#include "Resource/Scene.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#include "Editor/ActionManager.h"
namespace GALAXY 
{
	Component::Transform::Transform()
	{
	}

	Component::Transform::Transform(const Vec3f& position, const Quat& rotation, const Vec3f& scale /*= { 1, 1, 1 }*/)
	{

	}

	void Component::Transform::SetLocalPosition(const Vec3f& localPosition)
	{
		m_dirty = true;
		m_localPosition = localPosition;
	}

	void Component::Transform::SetLocalRotation(const Quat& localRotation)
	{
		m_dirty = true;
		m_localRotation = localRotation;
		m_localEulerRotation = localRotation.ToEuler();
	}

	void Component::Transform::SetLocalRotation(const Vec3f& localRotation)
	{
		m_dirty = true;
		m_localEulerRotation = localRotation;
		m_localRotation = localRotation.ToQuaternion();
	}

	void Component::Transform::SetLocalScale(const Vec3f& localScale)
	{
		m_dirty = true;
		m_localScale = localScale;
	}

	Vec3f Component::Transform::GetLocalPosition() const
	{
		return m_localPosition;
	}

	Quat Component::Transform::GetLocalRotation() const
	{
		return m_localRotation;
	}

	Vec3f Component::Transform::GetLocalEulerRotation() const
	{
		return m_localEulerRotation;
	}

	Vec3f Component::Transform::GetLocalScale() const
	{
		return m_localScale;
	}

	Mat4 Component::Transform::GetLocalMatrix() const
	{
		return Mat4::CreateTransformMatrix(m_localPosition, m_localRotation, m_localScale);
	}

	Vec3f Component::Transform::GetWorldPosition() const
	{
		if (gameObject.lock() && gameObject.lock()->GetParent().lock())
		{
			return gameObject.lock()->GetParent().lock()->Transform()->GetModelMatrix() * m_localPosition;
		}
		else
		{
			return m_localPosition;
		}
	}

	Quat Component::Transform::GetWorldRotation() const
	{
		if (gameObject.lock() && gameObject.lock()->GetParent().lock())
		{
			return GetModelMatrix().GetRotation();
		}
		else
		{
			return m_localRotation;
		}
	}

	Vec3f Component::Transform::GetUp() const
	{
		return GetWorldRotation() * Vec3f::Up();
	}

	Vec3f Component::Transform::GetRight() const
	{
		return GetWorldRotation() * Vec3f::Right();
	}

	Vec3f Component::Transform::GetForward() const
	{
		return GetWorldRotation() * Vec3f::Forward();
	}

	void Component::Transform::OnUpdate()
	{
		if (!m_dirty)
			return;
		ForceUpdate();
	}

	void Component::Transform::ForceUpdate()
	{
		if (gameObject.lock() && gameObject.lock()->GetParent().lock())
			ComputeModelMatrix(gameObject.lock()->GetParent().lock()->Transform()->GetModelMatrix());
		else
			ComputeModelMatrix();


		if (gameObject.lock())
		{
			for (auto& child : gameObject.lock()->GetChildren())
			{
				child.lock()->Transform()->ForceUpdate();
			}
		}
	}

	void Component::Transform::ComputeModelMatrix(const Mat4& parentMatrix)
	{
		m_modelMatrix = GetLocalMatrix() * parentMatrix;
		m_dirty = false;
	}

	void Component::Transform::ComputeModelMatrix()
	{
		m_modelMatrix = GetLocalMatrix();
		m_dirty = false;
	}

	Mat4 Component::Transform::GetModelMatrix() const
	{
		return m_modelMatrix;
	}

	Vec3f Component::Transform::GetWorldScale() const
	{
		if (gameObject.lock() && gameObject.lock()->GetParent().lock())
		{
			return GetModelMatrix().GetScale<float>();
		}
		else
		{
			return m_localScale;
		}
	}

	Vec3f Component::Transform::GetWorldEulerRotation() const
	{
		if (gameObject.lock() && gameObject.lock()->GetParent().lock())
		{
			return GetModelMatrix().GetEulerRotation<float>();
		}
		else
		{
			return m_localEulerRotation;
		}
	}


	void Component::Transform::ShowInInspector()
	{
		Vec3f position = m_localPosition;
		Vec3f rotation = m_localEulerRotation;
		Vec3f scale = m_localScale;

		/* Position Vec3 control */
		{
			static Vec3f previousPosition;
			static bool previousTrue = false;
			if (Wrapper::GUI::DrawVec3Control("Position", &position.x))
			{
				if (!previousTrue)
					previousPosition = m_localPosition;
				previousTrue = true;
			}
			else if (previousTrue && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				previousTrue = false;
				Editor::Action action(
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
			if (Wrapper::GUI::DrawVec3Control("Rotation", &rotation.x))
			{
				if (!previousTrue)
					previousRotation = m_localEulerRotation;
				previousTrue = true;
			}
			else if (previousTrue && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				previousTrue = false;
				Editor::Action action(
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
			if (Wrapper::GUI::DrawVec3Control("Scale", &scale.x, 1.f, true))
			{
				if (!previousTrue)
					previousScale = m_localScale;
				previousTrue = true;
			}
			else if (previousTrue && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				previousTrue = false;
				Editor::Action action(
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
	}

	void Component::Transform::Rotate(Vec3f axis, float angle, Space relativeTo /*= Space::Local*/)
	{
		if (relativeTo == Space::Local)
			RotateAround(TransformDirection(axis), angle);
		else
			RotateAround(axis, angle);
	}

	void Component::Transform::RotateAround(Vec3f point, Vec3f axis, float angle)
	{
		Quat q = Quat::AngleAxis(angle, axis);
		Vec3f dif = GetWorldPosition() - point;
		dif = q * dif;
		SetWorldPosition(point + dif);
		Quat worldRotation = GetWorldRotation();
		SetWorldRotation(worldRotation * worldRotation.GetInverse() * q * worldRotation);
	}

	void Component::Transform::RotateAround(Vec3f axis, float angle)
	{
		Quat rotation = Quat::AngleAxis(angle, axis);

		Vec3f pivot = GetWorldPosition();
		Vec3f relativePosition = Vec3f::Zero();

		// Rotate the relative position
		relativePosition = rotation * relativePosition;

		// Update the object's position
		SetWorldPosition(pivot + relativePosition);

		// Update the object's rotation
		Quat worldRotation = GetWorldRotation();
		Quat rot = worldRotation * worldRotation.GetInverse() * rotation * worldRotation;
		SetWorldRotation(rot);
	}

	Vec3f Component::Transform::TransformDirection(Vec3f direction)
	{
		return GetWorldRotation() * direction;
	}

	void Component::Transform::SetWorldPosition(const Vec3f& worldPosition)
	{
		//TODO :
		SetLocalPosition(worldPosition);
	}

	void Component::Transform::SetWorldRotation(const Quat& worldRotation)
	{
		//TODO :
		SetLocalRotation(worldRotation);
	}

	void Component::Transform::SetWorldRotation(const Vec3f& worldRotation)
	{
		//TODO :
		SetLocalRotation(worldRotation);
	}

	void Component::Transform::SetWorldScale(const Vec3f& worldScale)
	{
		//TODO :
		SetLocalScale(worldScale);
	}
}