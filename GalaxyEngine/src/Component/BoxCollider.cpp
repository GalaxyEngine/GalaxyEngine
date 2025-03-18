#include "pch.h"
#include "Component/BoxCollider.h"
#include "Component/RigidBody.h"

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

    Mat4 Component::BoxCollider::GetInverseInertia(float Mass) const
    {
        Vec3f Scale = m_size * GetTransform()->GetWorldScale();

        Vec3f fullWidth = Scale * 2;

        Vec3f dimsSqr = fullWidth * fullWidth;

        float inverseMass = 1.0f / Mass;
        Vec3f inverseInertia;
        inverseInertia.x = (12.0f * inverseMass) / (dimsSqr.y + dimsSqr.z);
        inverseInertia.y = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.z);
        inverseInertia.z = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.y);
        return Mat4::CreateScaleMatrix(inverseInertia);
    }

    void Component::BoxCollider::Serialize(CppSer::Serializer& serializer)
    {
        Collider::Serialize(serializer);
        serializer << CppSer::Pair::Key << "Size" << CppSer::Pair::Value << m_size;
    }

    void Component::BoxCollider::Deserialize(CppSer::Parser& parser)
    {
        Collider::Deserialize(parser);
        m_size = parser["Size"].As<Vec3f>();
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
        Vec4f color = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
        if (!Core::Application::IsEditorMode())
        {
            float isColliding = p_debugCollide ? 1.0f : 0.0f;
            float isAsleep = (!GetAttachedRigidbody().expired() && GetAttachedRigidbody().lock()->IsSleeping()) ? 1.0f : 0.0f;
            color = Vec4f(1.0f-isColliding, isColliding, isAsleep, 1.f);
        }
        // Wrapper::Renderer::GetInstance()->DrawCube(position, m_size * scale * 1.01f, rotation, Vec4f(1.0f-isColliding, isColliding, isAsleep, 0.2f));
        Wrapper::Renderer::GetInstance()->DrawWireCube(position, m_size * scale, rotation, color, 2.f);
    }
    
    void Component::BoxCollider::ShowInInspector()
    {
        Collider::ShowInInspector();
        ImGui::DragFloat3("Size", &m_size.x, 0.1f);
    }
#endif
}
