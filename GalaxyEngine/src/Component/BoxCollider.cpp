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

        // Convert quaternion to rotation matrix
        Mat4 rotMatrix = rotation.ToRotationMatrix();

        // Calculate new half-extents after rotation
        Vec3f newHe(
            he.x * std::abs(rotMatrix[0][0]) + he.y * std::abs(rotMatrix[0][1]) + he.z * std::abs(rotMatrix[0][2]),
            he.x * std::abs(rotMatrix[1][0]) + he.y * std::abs(rotMatrix[1][1]) + he.z * std::abs(rotMatrix[1][2]),
            he.x * std::abs(rotMatrix[2][0]) + he.y * std::abs(rotMatrix[2][1]) + he.z * std::abs(rotMatrix[2][2])
        );

        return {position, newHe, true};
    }

    Vec3f Component::BoxCollider::Support(const Vec3f& direction) {
        Vec3f worldPosition = p_gameObject->GetTransform()->GetWorldPosition();

        Quat rotation = p_gameObject->GetTransform()->GetWorldRotation();

        Vec3f scale = p_gameObject->GetTransform()->GetWorldScale() * m_size;

        // Transform the direction vector into the local space of the cube
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
