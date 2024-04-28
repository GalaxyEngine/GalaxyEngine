#include "pch.h"
#include "Component/BoxCollider.h"

#include "Core/GameObject.h"
#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
    void Component::BoxCollider::ShowInInspector()
    {
        ImGui::DragFloat3("Size", &m_size.x, 0.1f);
    }

    void Component::BoxCollider::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateBoxCollider(this);
    }

    void Component::BoxCollider::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyBoxCollider(this);
    }

    void Component::BoxCollider::OnEditorDraw()
    {
        auto position = p_gameObject->GetTransform()->GetWorldPosition();
        auto scale = p_gameObject->GetTransform()->GetWorldScale();
        Wrapper::Renderer::GetInstance()->DrawWireCube(position, m_size * scale, Vec4f(0, 1, 0, 1), 10.f);
    }
}
