#include "pch.h"
#include "Component/Listener.h"

#include "Core/GameObject.h"

#include "Wrapper/Audio.h"


namespace GALAXY 
{
    void Component::Listener::OnTransformUpdate()
    {
        auto worldPosition = p_gameObject->GetTransform()->GetWorldPosition();
        auto direction = p_gameObject->GetTransform()->GetForward();
        SetPosition(worldPosition);
        SetDirection(direction);
    }

    void Component::Listener::OnCreate()
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->AddListener(this);
        p_gameObject->GetTransform()->EOnUpdate.Bind(std::bind(&Listener::OnTransformUpdate, this));
    }

    void Component::Listener::OnDestroy()
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->RemoveListener(this);
    }

    void Component::Listener::SetPosition(const Vec3f& position)
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetListenerPosition(m_listenerIndex, position);
    }

    void Component::Listener::SetDirection(const Vec3f& direction)
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetListenerDirection(m_listenerIndex, direction);
    }
}
