#include "pch.h"
#include "Component/Rigidbody.h"

#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
#ifdef WITH_EDITOR
    void Component::Rigidbody::ShowInInspector()
    {
    }
#endif

    void Component::Rigidbody::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateRigidbody(this);
    }

    void Component::Rigidbody::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyRigidbody(this);
    }
}
