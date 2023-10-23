#include "pch.h"
#include "Editor/Gizmo.h"

#include "Core/Input.h"
#include "Core/GameObject.h"

#include "Component/Transform.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"

#include "EditorUI/EditorUIManager.h"
#include "EditorUI/SceneWindow.h"

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
      if (!m_object.lock() || !EditorUI::EditorUIManager::GetInstance()->GetSceneWindow()->IsHovered())
         return;

      if (Input::IsKeyPressed(Key::W)) {m_type = GizmoType::Translation; }
      if (Input::IsKeyPressed(Key::E)) {m_type = GizmoType::Rotation; }
      if (Input::IsKeyPressed(Key::R)) {m_type = GizmoType::Scale; }
      if (Input::IsKeyPressed(Key::Q)) {m_type = GizmoType::None; }

      if (Input::IsMouseButtonPressed(MouseButton::BUTTON_1))
      {
         Physic::Ray cameraRay = Render::Camera::GetEditorCamera()->ScreenPointToRay(Vec3f{Input::GetMousePosition(), 0});
         switch (m_type)
         {
            case GizmoType::Translation:
            {
               m_gizmoCenter = m_transform->GetWorldPosition();
               HandleAxis(cameraRay);
               break;
            }
            case GizmoType::Rotation:
            {
               break;
            }
            case GizmoType::Scale:
            {
               m_gizmoScale = m_transform->GetWorldScale();
               HandleAxis(cameraRay);
               break;
            }
         default:
            break;
         }
      }
      // TODO : Translate, Rotate, Scale
      return;
      if (Input::IsMouseButtonDown(MouseButton::BUTTON_1))
      {
         Physic::Ray cameraRay = Render::Camera::GetEditorCamera()->ScreenPointToRay(Vec3f{Input::GetMousePosition(), 0});
         switch (m_type)
         {
            case GizmoType::Translation:
            {
               int i = (int)m_axis;
               CalculateRayDistance(cameraRay, i);

               m_currentPosition = m_translateRays[i].origin + m_translateRays[i].direction * m_translateRays[i].scale;

               m_transform->SetWorldPosition(m_gizmoCenter - m_startPosition + m_currentPosition);
               break;
            }
            case GizmoType::Rotation:
            {
               break;
            }
            case GizmoType::Scale:
            {
               int i = (int)m_axis;
               CalculateRayDistance(cameraRay, i);

               m_currentPosition = m_translateRays[i].origin + m_translateRays[i].direction * m_translateRays[i].scale;

               // m_transform->SetWorldPosition(m_gizmoScale - m_startPosition + m_currentPosition);
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

      Component::Transform *transform = m_object.lock()->Transform();

      switch (m_type)
         {
            case GizmoType::Scale:
            case GizmoType::Translation:
            {
               m_renderer->SetDepthRange(0.00f, 0.01f);
               m_renderer->DrawLine(transform->GetWorldPosition(), transform->GetWorldPosition() + transform->GetRight() * m_gizmoLength, Vec4f(1, 0, 0, 1), 3.f);
               m_renderer->DrawLine(transform->GetWorldPosition(), transform->GetWorldPosition() + transform->GetUp() * m_gizmoLength, Vec4f(0, 1, 0, 1), 3.f);
               m_renderer->DrawLine(transform->GetWorldPosition(), transform->GetWorldPosition() + transform->GetForward() * m_gizmoLength, Vec4f(0, 0, 1, 1), 3.f);       
               m_renderer->SetDepthRange(0.01f, 1.f);
               break;
            }
            case GizmoType::Rotation:
            {
               break;
            }
         default:
            break;
         }

      // TODO : Translate, Rotate, Scale
   }


   void Editor::Gizmo::SetGameObject(Weak<Core::GameObject> object)
   {
		m_object = object;
		m_transform = m_object.lock()->Transform();
   }

   float ClosestDistanceBetweenLines(Physic::Ray& l1, Physic::Ray& l2)
   {
      Vec3f w0 = l1.origin - l2.origin; // vector from origin of l2 to origin of l1
      float a = l1.direction.Dot(l1.direction); // dot product of l1's direction with itself
      float b = l1.direction.Dot(l2.direction); // dot product of l1's direction with l2's direction
      float c = l2.direction.Dot(l2.direction); // dot product of l2's direction with itself
      float d = l1.direction.Dot(w0); // dot product of l1's direction with w0
      float e = l2.direction.Dot(w0); // dot product of l2's direction with w0
      float denom = a * c - b * b; // denominator of equation for t1 and t2
      if (denom == 0.0f)
         denom = 0.01f;
      l1.scale = (b * e - c * d) / denom; // parameter value for closest point on l1
      l2.scale = (a * e - b * d) / denom; // parameter value for closest point on l2
      Vec3f p1 = l1.origin + l1.direction * l1.scale; // closest point on l1
      Vec3f p2 = l2.origin + l2.direction * l2.scale; // closest point on l2
      return (p1 - p2).LengthSquared(); // distance between closest points
   }

   float Editor::Gizmo::CalculateRayDistance(Physic::Ray& mouseRay, int index)
   {
      Vec3f axisVector = Vec3f::Zero();
      axisVector[index] = 1;

      m_translateRays[index].origin = m_object.lock()->Transform()->GetWorldPosition();
      m_translateRays[index].direction = axisVector;

      return ClosestDistanceBetweenLines(mouseRay, m_translateRays[index]);
   }

   void Editor::Gizmo::HandleAxis(Physic::Ray &mouseRay)
   {
      float distance = FLT_MAX;
      float tmpDistance;

      for (int i = 0; i < 3; i++)
      {
         tmpDistance = CalculateRayDistance(mouseRay, i);

         if (tmpDistance < distance)
         {
            distance = tmpDistance;
            m_axis = (GizmoAxis)i;
            m_startPosition = m_translateRays[i].origin + m_translateRays[i].direction * m_translateRays[i].scale;
         }
      }
      PrintLog("Closest axis : %d", (int)m_axis);
      if (m_startPosition.Distance(m_object.lock()->Transform()->GetWorldPosition()) > m_gizmoLength || distance > 1.f)
      {   
         m_axis = GizmoAxis::None;
         m_gizmoClicked = false;
      }
      else
      {
         m_gizmoClicked = true;
      }
   }
}
