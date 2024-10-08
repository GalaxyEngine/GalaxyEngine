#include "pch.h"
#include "Component/Rigidbody.h"

#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
    void Component::Rigidbody::ShowInInspector()
    {
#ifdef WITH_EDITOR
#endif
    }

    void Component::Rigidbody::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateRigidbody(this);
    }

    void Component::Rigidbody::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyRigidbody(this);
    }
}
