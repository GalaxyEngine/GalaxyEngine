#include "pch.h"
#include "Component/BoxCollider.h"

#include "Core/GameObject.h"
#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
    void Component::BoxCollider::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateBoxCollider(weak_from_this());
    }

    void Component::BoxCollider::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyBoxCollider(weak_from_this());
    }
    
    Physic::AABB Component::BoxCollider::GetAABB()
    {
        Vec3f position = p_gameObject->GetTransform()->GetWorldPosition();
        Quat rotation = p_gameObject->GetTransform()->GetWorldRotation();
        Vec3f scale = p_gameObject->GetTransform()->GetWorldScale();

        Vec3f he = m_size * scale; // Original half-extents

        Vec3f result = Vec3f();

        for (int i = 0; i < 8; i++)
        {
            Vec3f e;
            e.x = (i & 0x1) ? 1.0f : -1.0f;
            e.y = (i & 0x2) ? 1.0f : -1.0f;
            e.z = (i & 0x4) ? 1.0f : -1.0f;

            e *= he;
            e = rotation * e;

            for (int j = 0; j < 3; j++)
            {
                result[j] = fmaxf(fabsf(e[j]), result[j]);
            }
        }

        return {position, result, true};
    }

    Vec3f Component::BoxCollider::Support(const Vec3f& direction)
    {
        Vec3f worldPosition = p_gameObject->GetTransform()->GetWorldPosition();

        Quat rotation = p_gameObject->GetTransform()->GetWorldRotation();

        Vec3f scale = p_gameObject->GetTransform()->GetWorldScale() * m_size;

        Vec3f localDirection = rotation.GetInverse() * direction;
        // Find the farthest point in the local space
        Vec3f localSupport;
        localSupport.x = (localDirection.x > 0) ? scale.x : -scale.x;
        localSupport.y = (localDirection.y > 0) ? scale.y : -scale.y;
        localSupport.z = (localDirection.z > 0) ? scale.z : -scale.z;

        Vec3f worldSupport = worldPosition + rotation * localSupport;
        
        return worldSupport;
    }

#ifdef WITH_EDITOR
    void Component::BoxCollider::OnEditorDraw()
    {
        Collider::OnEditorDraw();
        if (!p_gameObject->IsSelected())
            return;
        Vec3f position = GetTransform()->GetWorldPosition();
        Quat rotation = GetTransform()->GetWorldRotation();
        Vec3f scale = GetTransform()->GetWorldScale();
        Wrapper::Renderer::GetInstance()->DrawWireCube(position, m_size * scale, rotation, p_debugCollide ? Vec4f(1, 0, 0, 1) : Vec4f(0, 1, 0, 1), 10.f);
    }
    
    void Component::BoxCollider::ShowInInspector()
    {
        Collider::ShowInInspector();
        ImGui::DragFloat3("Size", &m_size.x, 0.1f);
    }
#endif
}
