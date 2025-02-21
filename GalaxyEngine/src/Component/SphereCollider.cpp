#include "pch.h"
#include "Component/SphereCollider.h"

#include "Core/GameObject.h"
#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
#ifdef WITH_EDITOR
    void Component::SphereCollider::ShowInInspector()
    {
        Collider::ShowInInspector();
        ImGui::DragFloat("Size", &m_size, 0.1f);
    }
    
    void Component::SphereCollider::OnEditorDraw()
    {
        Collider::OnEditorDraw();
        if (!p_gameObject->IsSelected())
            return;
        auto position = p_gameObject->GetTransform()->GetWorldPosition();
        auto scale = p_gameObject->GetTransform()->GetWorldScale();
        float s = fmaxf(fmaxf(scale.x, scale.y), scale.z) * m_size;
        Vec4f color = p_debugCollide ? Vec4f(1, 0, 0, 1) : Vec4f(0, 1, 0, 1);
        Wrapper::Renderer::GetInstance()->DrawSimpleWireSphere(position, s, 32, color, 1.f);
        // Wrapper::Renderer::GetInstance()->DrawWireSphere(position, s, 16, 8, color, 2.f);
    }
#endif

    void Component::SphereCollider::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateSphereCollider(weak_from_this());
    }

    void Component::SphereCollider::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroySphereCollider(weak_from_this());
    }

    Physic::AABB Component::SphereCollider::GetAABB()
    {
        Vec3f position = p_gameObject->GetTransform()->GetWorldPosition();
        Vec3f scale = p_gameObject->GetTransform()->GetWorldScale();
        return {position, scale * m_size, true};
    }

    Vec3f Component::SphereCollider::Support(const Vec3f& direction)
    {
        Vec3f worldPosition = p_gameObject->GetTransform()->GetWorldPosition();
        Vec3f scale = p_gameObject->GetTransform()->GetWorldScale();

        // Normalize the direction vector to ensure it has a length of 1
        Vec3f normalizedDirection = direction.GetNormalize();

        // Calculate the farthest point on the sphere in the given direction
        Vec3f supportPoint = worldPosition + normalizedDirection * scale * m_size;

        return supportPoint;
    }

    void Component::SphereCollider::Serialize(CppSer::Serializer& serializer)
    {
        Collider::Serialize(serializer);
        serializer << CppSer::Pair::Key << "Size" << CppSer::Pair::Value << m_size;
    }

    void Component::SphereCollider::Deserialize(CppSer::Parser& parser)
    {
        Collider::Deserialize(parser);
        m_size = parser["Size"].As<float>();
    }

    Mat4 Component::SphereCollider::GetInverseInertia(float Mass) const
    {
        auto transform = GetTransform();
        float inverseMass = 1.f / Mass;
        float radius	= (transform->GetWorldScale() * m_size).x;
        float i			= 2.5f * inverseMass / (radius*radius);

        return Mat4::CreateScaleMatrix(Vec3f(i));
    }
}
