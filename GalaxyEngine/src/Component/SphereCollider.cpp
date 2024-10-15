#include "pch.h"
#include "Component/SphereCollider.h"

#include "Core/GameObject.h"
#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
    void Component::SphereCollider::ShowInInspector()
    {
        ImGui::DragFloat("Size", &m_size, 0.1f);
    }

    void Component::SphereCollider::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateSphereCollider(this);
    }

    void Component::SphereCollider::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroySphereCollider(this);
    }

    void Component::SphereCollider::OnEditorDraw()
    {
        if (!p_gameObject->IsSelected())
            return;
        auto position = p_gameObject->GetTransform()->GetWorldPosition();
        auto scale = p_gameObject->GetTransform()->GetWorldScale();
        float s = fmaxf(fmaxf(scale.x, scale.y), scale.z);
        Wrapper::Renderer::GetInstance()->DrawWireCircle(position, Vec3f::Up(), s, 32, Vec4f(0, 1, 0, 1));
        Wrapper::Renderer::GetInstance()->DrawWireCircle(position, Vec3f::Right(), s, 32, Vec4f(0, 1, 0, 1));
        Wrapper::Renderer::GetInstance()->DrawWireCircle(position, Vec3f::Forward(), s, 32, Vec4f(0, 1, 0, 1));
    }
}
