#include "pch.h"
#include "Editor/Gizmo.h"

#include "Core/Input.h"
#include "Core/GameObject.h"

#include "Component/Transform.h"

namespace GALAXY
{
   Editor::Gizmo::Gizmo()
   {
      m_renderer = Wrapper::Renderer::GetInstance();
   }

   Editor::Gizmo::~Gizmo()
   {
   }

   void Editor::Gizmo::Update()
   {
	   if (!m_object.lock())
		   return;
	   Component::Transform* transform = m_object.lock()->Transform();

      if (Input::IsMouseButtonPressed(MouseButton::BUTTON_1))
      {
         
      }
	   //TODO : Translate, Rotate, Scale
   }

   void Editor::Gizmo::Draw()
   {
	   if (!m_object.lock())
		   return;
      Component::Transform* transform = m_object.lock()->Transform();
      m_renderer->DrawLine(transform->GetWorldPosition(), transform->GetWorldPosition() + transform->GetRight() * 100.f, Vec4f(1, 0, 0, 1), 3.f);
      m_renderer->DrawLine(transform->GetWorldPosition(), transform->GetWorldPosition() + transform->GetUp() * 100.f, Vec4f(0, 1, 0, 1), 3.f);
	   m_renderer->DrawLine(transform->GetWorldPosition(), transform->GetWorldPosition() + transform->GetForward() * 100.f, Vec4f(0, 0, 1, 1), 3.f);

	  //TODO : Translate, Rotate, Scale
   }
}
