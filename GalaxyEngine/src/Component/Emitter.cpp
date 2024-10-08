#include "pch.h"
#include "Component/Emitter.h"

#include "Core/GameObject.h"

#include "Resource/ResourceManager.h"

#include "Utils/Time.h"

#include "Wrapper/Audio.h"

namespace GALAXY 
{
    void Component::Emitter::ShowInInspector()
    {
#ifdef WITH_EDITOR
        Vec2f buttonSize = { ImGui::GetContentRegionAvail().x, 0 };
        if (ImGui::Button(m_sound.lock() ? m_sound.lock()->GetFileInfo().GetFileName().c_str() : "Empty", buttonSize))
        {
            ImGui::OpenPopup("SoundPopup");
        }
        Weak<Resource::Sound> sound;
        if (Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::Sound>("SoundPopup", sound))
        {
            SetSound(sound.lock());
        }
        if (ImGui::InputFloat("Volume", &m_volume, 0.0f, 1.0f))
        {
            SetVolume(m_volume);
        }

        float currentTime = 0.f;
        auto audioInstance = Wrapper::Audio::GetInstance();
        if (m_sound.lock())
        {
            currentTime = audioInstance->GetPlaybackTime(GetGameObject()->GetUUID());
        }
        std::string format = Utils::Time::FormatTimeSeconds(currentTime)
        + "/" + Utils::Time::FormatTimeSeconds(m_duration);

        if (ImGui::SliderFloat("##Time", &currentTime, 0.0f, m_duration, format.c_str()))
        {
            audioInstance->SetPlaybackTime(GetGameObject()->GetUUID(), currentTime);
        }

        bool hasSound = m_sound.lock() != nullptr;
        
        bool isPlaying = hasSound && audioInstance->IsPlaying(GetGameObject()->GetUUID());
        if (!isPlaying)
        {
            if (ImGui::Button("Play", buttonSize) && hasSound)
            {
                Play();
            }
        }
        else
        {
            if (ImGui::Button("Stop", buttonSize)&& hasSound)
            {
                Stop();
            }
        }

        ImGui::SeparatorText("Parameters");
        int attenuationModel = (int)m_attenuationModel;
        if (ImGui::Combo("Attenuation Model", &attenuationModel, SerializeAttenuationModelEnum())&& hasSound)
        {
            SetAttenuationModel((AttenuationModel)attenuationModel);
        }

        if (ImGui::InputFloat("Min Distance", &m_minDistance, 0.0f, 100.0f)&& hasSound)
        {
            SetMinDistance(m_minDistance);
        }
        if (ImGui::InputFloat("Max Distance", &m_maxDistance, 0.0f, 100.0f)&& hasSound)
        {
            SetMaxDistance(m_maxDistance);
        }
        if (ImGui::InputFloat("Pitch", &m_pitch, 0.0f, 100.0f)&& hasSound)
        {
            SetPitch(m_pitch);
        }
        if (ImGui::InputFloat("Pan", &m_pan, -1.0f, 1.0f)&& hasSound)
        {
            SetPan(m_pan);
        }
        if (ImGui::Checkbox("Looping", &m_isLooping)&& hasSound)
        {
            SetLooping(m_isLooping);
        }
        if (ImGui::InputFloat("Doppler Factor", &m_dopplerFactor, 0.0f, 100.0f)&& hasSound)
        {
            SetDopplerFactor(m_dopplerFactor);
        }
#endif
    }

    void Component::Emitter::OnTransformUpdate()
    {
        if (m_sound.lock())
        {
            auto audioInstance = Wrapper::Audio::GetInstance();
            SetPosition(p_gameObject->GetTransform()->GetWorldPosition());
            SetDirection(p_gameObject->GetTransform()->GetForward());
        }
    }
    void Component::Emitter::OnCreate()
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->AddEmitter(this);
        GetGameObject()->GetTransform()->EOnUpdate.Bind(std::bind(&Emitter::OnTransformUpdate, this));
    }
    
    void Component::Emitter::OnDestroy()
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->RemoveEmitter(this);
    }

    void Component::Emitter::Play()
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->PlayEmitter(GetGameObject()->GetUUID());
    }

    void Component::Emitter::Stop()
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->StopEmitter(GetGameObject()->GetUUID());
    }

    void Component::Emitter::SetPosition(const Vec3f& position)
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetEmitterPosition(p_gameObject->GetUUID(), position);
    }

    void Component::Emitter::SetDirection(const Vec3f& direction)
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetEmitterDirection(p_gameObject->GetUUID(), direction);
    }

    void Component::Emitter::SetSound(Shared<Resource::Sound> sound)
    {
        bool same = m_sound.lock() == sound;
        m_sound = sound;
        if (!same && sound)
        {
            auto audioInstance = Wrapper::Audio::GetInstance();
            audioInstance->UpdateEmitterSound(this);
            m_duration = audioInstance->GetSoundDuration(GetGameObject()->GetUUID());
        }
    }

    void Component::Emitter::SetVolume(float volume)
    {
        m_volume = volume;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetEmitterVolume(p_gameObject->GetUUID(), volume);
    }

    void Component::Emitter::SetPitch(float pitch)
    {
        m_pitch = pitch;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetPitch(p_gameObject->GetUUID(), pitch);
    }

    void Component::Emitter::SetMinDistance(float minDistance)
    {
        m_minDistance = minDistance;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetMinDistance(p_gameObject->GetUUID(), minDistance);
    }

    void Component::Emitter::SetMaxDistance(float maxDistance)
    {
        m_maxDistance = maxDistance;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetMaxDistance(p_gameObject->GetUUID(), maxDistance);
    }

    void Component::Emitter::SetAttenuationModel(AttenuationModel attenuationModel)
    {
        m_attenuationModel = attenuationModel;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetAttenuationModel(p_gameObject->GetUUID(), attenuationModel);
    }

    void Component::Emitter::SetLooping(bool isLooping)
    {
        m_isLooping = isLooping;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetLooping(p_gameObject->GetUUID(), isLooping);
    }

    void Component::Emitter::SetDopplerFactor(float dopplerFactor)
    {
        m_dopplerFactor = dopplerFactor;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetDopplerFactor(p_gameObject->GetUUID(), dopplerFactor);
    }

    void Component::Emitter::SetPan(float pan)
    {
        m_pan = pan;
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->SetPan(p_gameObject->GetUUID(), pan);
    }

    void Component::Emitter::Serialize(CppSer::Serializer& serializer)
    {
        if (m_sound.lock())
            serializer << CppSer::Pair::Key << "Sound" << CppSer::Pair::Value << m_sound.lock()->GetUUID();
        else
            serializer << CppSer::Pair::Key << "Sound" << CppSer::Pair::Value << UUID_NULL;

        serializer << CppSer::Pair::Key << "Volume" << CppSer::Pair::Value << m_volume;
        serializer << CppSer::Pair::Key << "Pitch" << CppSer::Pair::Value << m_pitch;
        serializer << CppSer::Pair::Key << "Min Distance" << CppSer::Pair::Value << m_minDistance;
        serializer << CppSer::Pair::Key << "Max Distance" << CppSer::Pair::Value << m_maxDistance;
        serializer << CppSer::Pair::Key << "Attenuation Model" << CppSer::Pair::Value << (int)m_attenuationModel;
        serializer << CppSer::Pair::Key << "Looping" << CppSer::Pair::Value << m_isLooping;
        serializer << CppSer::Pair::Key << "Doppler Factor" << CppSer::Pair::Value << m_dopplerFactor;
        serializer << CppSer::Pair::Key << "Pan" << CppSer::Pair::Value << m_pan;
    }

    void Component::Emitter::Deserialize(CppSer::Parser& parser)
    {
        auto uuid = parser["Sound"].As<uint64_t>();
        auto sound = Resource::ResourceManager::GetOrLoad<Resource::Sound>(uuid);

        m_volume = parser["Volume"].As<float>();
        m_pitch = parser["Pitch"].As<float>();
        m_minDistance = parser["Min Distance"].As<float>();
        m_maxDistance = parser["Max Distance"].As<float>();
        m_attenuationModel = static_cast<AttenuationModel>(parser["Attenuation Model"].As<int>());
        m_isLooping = parser["Looping"].As<bool>();
        m_dopplerFactor = parser["Doppler Factor"].As<float>();
        m_pan = parser["Pan"].As<float>();
        m_sound = sound.lock();
    }

    void Component::Emitter::AfterLoad()
    {
        auto audioInstance = Wrapper::Audio::GetInstance();
        audioInstance->UpdateEmitterSound(this);
        m_duration = audioInstance->GetSoundDuration(GetGameObject()->GetUUID());
    }
}
