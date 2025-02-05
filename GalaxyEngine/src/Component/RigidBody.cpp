#include "pch.h"
#include "Component/RigidBody.h"

#include "Core/Application.h"
#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
#ifdef WITH_EDITOR
    void Component::RigidBody::ShowInInspector()
    {
        ImGui::DragFloat("Mass", &m_mass, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Drag", &m_drag, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Angular Drag", &m_angularDrag, 0.1f, 0.0f, 100.0f);
        ImGui::SeparatorText("Force");
        if (Core::Application::IsPlayMode() || Core::Application::IsPauseMode())
        {
            ImGui::DragFloat3("Velocity", &m_velocity.x, 0.1f, -100.0f, 100.0f);
            ImGui::DragFloat3("Angular Velocity", &m_angularVelocity.x, 0.1f, -100.0f, 100.0f);
        }
        ImGui::DragFloat3("Gravity Force", &m_gravityForce.x, 0.1f, -100.0f, 100.0f);
    }
#endif

    void Component::RigidBody::Serialize(CppSer::Serializer& serializer)
    {
        serializer << CppSer::Pair::Key << "Mass" << CppSer::Pair::Value << m_mass;
        serializer << CppSer::Pair::Key << "Drag" << CppSer::Pair::Value << m_drag;
        serializer << CppSer::Pair::Key << "Angular Drag" << CppSer::Pair::Value << m_angularDrag;
        serializer << CppSer::Pair::Key << "Gravity Force" << CppSer::Pair::Value << m_gravityForce;
    }

    void Component::RigidBody::Deserialize(CppSer::Parser& parser)
    {
        SetMass(parser["Mass"].As<float>());
        SetDrag(parser["Drag"].As<float>());
        SetAngularDrag(parser["Angular Drag"].As<float>());
        SetGravityForce(parser["Gravity Force"].As<Vec3f>());
    }

    void Component::RigidBody::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateRigidBody(weak_from_this());
    }

    void Component::RigidBody::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyRigidBody(weak_from_this());
    }

    void Component::RigidBody::AddForce(const Vec3f& force)
    {
        Wrapper::PhysicsWrapper::GetInstance()->AddForce(weak_from_this(), force);
    }
}
