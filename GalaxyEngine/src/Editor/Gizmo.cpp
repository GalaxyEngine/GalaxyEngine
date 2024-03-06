#include "pch.h"

#include "Editor/Gizmo.h"
#include "Editor/Action.h"
#include "Editor/ActionManager.h"

#include "Core/Input.h"
#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#include "Component/Transform.h"

#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Editor/EditorCamera.h"

#include "Editor/UI/EditorUIManager.h"
#include "Editor/UI/SceneWindow.h"

#include "Physic/Plane.h"

#pragma region MathMethods
float ClosestDistanceBetweenLines(Physic::Ray& l1, Physic::Ray& l2)
{
	Vec3f w0 = l1.origin - l2.origin;
	float a = l1.direction.Dot(l1.direction);
	float b = l1.direction.Dot(l2.direction);
	float c = l2.direction.Dot(l2.direction);
	float d = l1.direction.Dot(w0);
	float e = l2.direction.Dot(w0);
	float denom = a * c - b * b;
	if (denom == 0.0f)
		denom = 0.01f;
	l1.scale = (b * e - c * d) / denom;
	l2.scale = (a * e - b * d) / denom;
	Vec3f p1 = l1.origin + l1.direction * l1.scale;
	Vec3f p2 = l2.origin + l2.direction * l2.scale;
	return (p1 - p2).LengthSquared();
}

bool RayCircleIntersection(const Physic::Ray& ray, Circle circle, Vec3f& intersectionPoint)
{
	// Calculate the intersection point between the ray and the plane of the circle
	float t = circle.orientation.Dot(circle.center - ray.origin) / circle.orientation.Dot(ray.direction);
	intersectionPoint = ray.origin + ray.direction * t;

	// Calculate the distance between the intersection point and the center of the circle
	float distance = intersectionPoint.Distance(circle.center);

	// Check if the distance is less than or equal to the radius of the circle
	return distance <= circle.radius;
}

Vec3f RayPlaneIntersection(const Physic::Ray& ray, Physic::Plane plane)
{
	Vec3f planePoint = -plane.normal * plane.distance; // calculate a point on the plane
	float dotProduct = ray.direction.Dot(plane.normal);

	if (dotProduct == 0.f) // ray and plane are parallel
	{
		return {};
	}

	float distance = plane.normal.Dot(planePoint - ray.origin) / dotProduct;

	if (distance < 0.f) // intersection behind the ray origin
	{
		return {};
	}

	return ray.origin + ray.direction * distance;
}

float ClosestDistanceLineCircle(const Physic::Ray& line, const Circle& c, Vec3f& point, bool enablePlane = false)
{
	Physic::Plane f = Physic::Plane(c.center, c.orientation);
	point = Vec3f::Zero();

	Vec3f intersection;
	if (RayCircleIntersection(line, c, intersection))
	{
		// project that point on to the circle's circumference
		point = c.center + (intersection - c.center).GetNormalize() * c.radius;
		return (intersection - point).Length();
	}
	else if (enablePlane)
	{
		point = RayPlaneIntersection(line, f);

		return point.Distance(line.origin);
	}
	return FLT_MAX;
}

float Angle(Vec3f from, Vec3f to)
{
	float denominator = (float)std::sqrt(from.LengthSquared() * to.LengthSquared());
	if (denominator < 1e-15F)
		return 0.f;

	float dot = std::clamp(from.Dot(to) / denominator, -1.f, 1.f);
	return ((float)std::acos(dot)) * RadToDeg;
}


Vec3f Project(Vec3f vector, Vec3f onNormal)
{
	float sqrMag = onNormal.Dot(onNormal);
	if (sqrMag < 1.401298E-45)
		return {};
	else
	{
		float dot = vector.Dot(onNormal);
		return Vec3f(onNormal.x * dot / sqrMag,
			onNormal.y * dot / sqrMag,
			onNormal.z * dot / sqrMag);
	}
}

float AngleBetweenVectorsInAxis(Vec3f vector1, Vec3f vector2, Vec3f axis)
{
	// Calculate the angle between the two vectors in the plane perpendicular to the custom axis
	float angleInPlane = Angle(vector1 - Project(vector1, axis),
		vector2 - Project(vector2, axis));

	// Calculate the sign of the angle using the cross product of the two vectors and the custom axis
	Vec3f crossProduct = vector1.Cross(vector2);
	float sign = crossProduct.Dot(axis) >= 0 ? 1.f : -1.f;

	// Calculate the final angle using the sign and the angle in the plane perpendicular to the custom axis
	float angle = sign * angleInPlane;

	return angle;
}
#pragma endregion

namespace GALAXY
{
	Editor::Gizmo::Gizmo()
	{
		m_renderer = Wrapper::Renderer::GetInstance();
	}

	Editor::Gizmo::~Gizmo()
	{
	}

	void Editor::Gizmo::Initialize()
	{
	}

	void Editor::Gizmo::Update()
	{
		//TODO : Fix bug on scaling when the object is rotated
		if (!m_object.lock())
			return;

		Editor::UI::SceneWindow* sceneWindow = Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow();

		if (!Render::Camera::GetEditorCamera()->IsLooking() && sceneWindow->IsHovered())
		{
			if (Input::IsKeyPressed(Key::W)) { m_type = GizmoType::Translation; }
			if (Input::IsKeyPressed(Key::E)) { m_type = GizmoType::Rotation; }
			if (Input::IsKeyPressed(Key::R)) { m_type = GizmoType::Scale; }
			if (Input::IsKeyPressed(Key::Q)) { m_type = GizmoType::None; }
		}

		if (m_type == GizmoType::None)
			return;

		Vec3f position = m_transform->GetWorldPosition();
		Quat rotation = m_transform->GetWorldRotation();
		Vec3f scale = m_transform->GetWorldScale();

		m_gizmoLength = Render::Camera::GetEditorCamera()->GetTransform()->GetLocalPosition().Distance(position) * 0.2f;

		// Update each frame even if mouse is not clicked
		switch (m_type)
		{
		case Editor::GizmoType::Translation:
		{
			for (size_t i = 0; i < 3; i++)
			{
				Vec3f axisVector = Vec3f::Zero();
				axisVector[i] = 1;

				m_translateRays[i].origin = position;

				if (m_mode == Space::Local)
					m_translateRays[i].direction = rotation * axisVector;
				else if (m_mode == Space::World)
					m_translateRays[i].direction = axisVector;
			}
		}
		break;
		case Editor::GizmoType::Rotation:
		{
			for (size_t i = 0; i < 3; i++)
			{
				Vec3f axisVector = Vec3f();
				axisVector[i] = 1;

				m_rotateCircle[i].center = position;

				if (m_mode == Space::Local)
					m_rotateCircle[i].orientation = rotation * axisVector;
				else if (m_mode == Space::World)
					m_rotateCircle[i].orientation = axisVector;

				m_rotateCircle[i].radius = m_gizmoLength;
			}
		}
		break;
		case Editor::GizmoType::Scale:
		{
			for (size_t i = 0; i < 3; i++)
			{
				Vec3f axisVector = Vec3f::Zero();
				axisVector[i] = 1;

				m_translateRays[i].origin = position;
				
				m_translateRays[i].direction = rotation * axisVector;
			}
		}
		break;
		default:
			break;
		}

		if (Input::IsMouseButtonReleased(MouseButton::BUTTON_1)) {
			if (m_gizmoClicked)
			{
				HandleAction();
			}
			m_gizmoClicked = false;
		}

		if (!sceneWindow->IsHovered())
			return;

		if (Input::IsMouseButtonPressed(MouseButton::BUTTON_1))
		{
			Physic::Ray cameraRay = Render::Camera::GetEditorCamera()->ScreenPointToRay(sceneWindow->GetMousePosition());
			switch (m_type)
			{
			case GizmoType::Translation:
			{
				m_gizmoCenter = position;
				HandleAxis(cameraRay);
				break;
			}
			case GizmoType::Rotation:
			{
				m_gizmoRotation = rotation;
				HandleRotation(cameraRay, position);
				break;
			}
			case GizmoType::Scale:
			{
				m_gizmoScale = scale;
				HandleAxis(cameraRay);
				break;
			}
			default:
				break;
			}
		}

		if (Input::IsMouseButtonDown(MouseButton::BUTTON_1) && m_gizmoClicked)
		{
			Physic::Ray cameraRay = Render::Camera::GetEditorCamera()->ScreenPointToRay(sceneWindow->GetMousePosition());
			switch (m_type)
			{
			case GizmoType::Translation:
			{
				int i = (int)m_axis;
				ClosestDistanceBetweenLines(cameraRay, m_translateRays[i]);

				m_currentPosition = m_translateRays[i].origin + m_translateRays[i].direction * m_translateRays[i].scale;

				m_transform->SetWorldPosition(m_gizmoCenter - m_startPosition + m_currentPosition);
				break;
			}
			case GizmoType::Rotation:
			{
				int i = (int)m_axis;

				Vec3f axisVector = Vec3f();
				axisVector[i] = 1;

				ClosestDistanceLineCircle(cameraRay, m_rotateCircle[i], m_currentPosition, true);

				float angle = -AngleBetweenVectorsInAxis(m_currentPosition - m_rotateCircle[i].center, m_startPosition - m_rotateCircle[i].center, m_mode == Space::Local ? m_gizmoRotation * axisVector : axisVector);

				if (m_mode == Space::Local)
					m_transform->SetWorldRotation(m_gizmoRotation * Quat::AngleAxis(angle, axisVector));
				else if (m_mode == Space::World)
					m_transform->SetWorldRotation(Quat::AngleAxis(angle, axisVector) * m_gizmoRotation);

				break;
			}
			case GizmoType::Scale:
			{
				int i = (int)m_axis;
				ClosestDistanceBetweenLines(cameraRay, m_translateRays[i]);

				Quat localRotation = m_transform->GetLocalRotation().GetInverse();

				m_currentPosition = m_translateRays[i].origin + m_translateRays[i].direction * m_translateRays[i].scale;

				Vec3f NewScale = m_gizmoScale - localRotation * (m_startPosition - m_currentPosition);

				Vec3f axisVector = Vec3f();
				axisVector[i] = NewScale[i];
				axisVector[(i + 1) % 3] = m_gizmoScale[(i + 1) % 3];
				axisVector[(i + 2) % 3] = m_gizmoScale[(i + 2) % 3];

				m_transform->SetWorldScale(axisVector);
				break;
			}
			default:
				break;
			}
		}
	}

	void Editor::Gizmo::Draw()
	{
		if (!m_object.lock())
			return;

		m_renderer->SetDepthRange(0.00f, 0.01f);
		switch (m_type)
		{
		case GizmoType::Scale:
		{
			Quat rotation = m_transform->GetWorldRotation();
			Vec3f cubeScale = Vec3f(m_gizmoLength) * 0.1f;
			for (size_t i = 0; i < 3; i++)
			{
				Vec4f color(0, 0, 0, 1);
				color[i] = 1;
				m_renderer->DrawWireCube(m_translateRays[i].origin + m_translateRays[i].direction * m_gizmoLength, cubeScale, rotation, color, 3.f);
				m_renderer->DrawLine(m_translateRays[i].origin, m_translateRays[i].origin + m_translateRays[i].direction * m_gizmoLength, color, 3.f);
			}
			break;
		}
		case GizmoType::Translation:
		{
			for (size_t i = 0; i < 3; i++)
			{
				Vec4f color(0, 0, 0, 1);
				color[i] = 1;
				m_renderer->DrawLine(m_translateRays[i].origin, m_translateRays[i].origin + m_translateRays[i].direction * m_gizmoLength, color, 3.f);
			}
			break;
		}
		case GizmoType::Rotation:
		{
			for (size_t i = 0; i < 3; i++)
			{
				Vec4f color(0, 0, 0, 1);
				color[i] = 1;
				m_renderer->DrawWireCircle(m_rotateCircle[i].center, m_rotateCircle[i].orientation, m_gizmoLength, 32, color, 3.f);
			} 
			if (m_gizmoClicked)
			{
				Vec3f position = m_object.lock()->GetTransform()->GetWorldPosition();
				m_renderer->DrawLine(position, m_currentPosition, Vec4f(1), 3.f);
				m_renderer->DrawLine(position, m_startPosition, Vec4f(1), 3.f);
			}
			break;
		}
		default:
			break;
		}

		if (m_type != GizmoType::None && m_gizmoClicked)
		{
			m_renderer->DrawWireCube(m_startPosition, Vec3f(0.01f) * m_gizmoLength, Vec4f(1.f, 1.f, 0.f, 1.f), 4.f);
			m_renderer->DrawWireCube(m_currentPosition, Vec3f(0.01f) * m_gizmoLength, Vec4f(1.f), 4.f);
		}

		m_renderer->SetDepthRange(0.01f, 1.f);
	}


	void Editor::Gizmo::SetGameObject(Weak<Core::GameObject> object)
	{
		m_object = object;
		if (m_object.lock())
			m_transform = m_object.lock()->GetTransform();
	}

	void Editor::Gizmo::HandleAxis(Physic::Ray& mouseRay)
	{
		float distance = FLT_MAX;
		float tmpDistance;

		for (int i = 0; i < 3; i++)
		{
			tmpDistance = ClosestDistanceBetweenLines(mouseRay, m_translateRays[i]);

			if (tmpDistance < distance)
			{
				distance = tmpDistance;
				m_axis = (GizmoAxis)i;
				m_startPosition = m_translateRays[i].origin + m_translateRays[i].direction * m_translateRays[i].scale;
			}
		}

		if (m_startPosition.Distance(m_object.lock()->GetTransform()->GetWorldPosition()) > m_gizmoLength || distance > 0.25f)
		{
			m_axis = GizmoAxis::None;
			m_gizmoClicked = false;
		}
		else
		{
			m_gizmoClicked = true;
		}
	}

	void Editor::Gizmo::HandleRotation(Physic::Ray& mouseRay, const Vec3f& position)
	{
		float distance = FLT_MAX;
		float tmpDistance;
		Vec3f tempPoint;

		for (int i = 0; i < 3; i++)
		{
			tmpDistance = ClosestDistanceLineCircle(mouseRay, m_rotateCircle[i], tempPoint);

			if (tmpDistance < distance)
			{
				m_startPosition = tempPoint;
				distance = tmpDistance;
				m_axis = (GizmoAxis)i;
			}
		}

		if (distance > 1.f)
		{
			m_axis = GizmoAxis::None;
			m_gizmoClicked = false;
		}
		else
		{
			m_gizmoClicked = true;
		}
	}

	void Editor::Gizmo::HandleAction()
	{
		Editor::Action action;
		switch (m_type)
		{
		case GizmoType::Translation:
		{
			action.Bind(
				[this, position = m_transform->GetWorldPosition()]()
				{
					m_transform->SetWorldPosition(position);
				},
				[this, position = m_gizmoCenter]()
				{
					m_transform->SetWorldPosition(position);
				});
			break;
		}
		case GizmoType::Rotation:
		{
			action.Bind(
				[this, rotation = m_transform->GetWorldRotation()]()
				{
					m_transform->SetWorldRotation(rotation);
				},
				[this, rotation = m_gizmoRotation]()
				{
					m_transform->SetWorldRotation(rotation);
				});
			break;
		}
		case GizmoType::Scale:
		{
			action.Bind(
				[this, rotation = m_transform->GetWorldScale()]()
				{
					m_transform->SetWorldScale(rotation);
				},
				[this, rotation = m_gizmoScale]()
				{
					m_transform->SetWorldScale(rotation);
				});
			break;
		}
		default:
			break;
		}
		Core::SceneHolder::GetCurrentScene()->GetActionManager()->AddAction(action);
	}

}
