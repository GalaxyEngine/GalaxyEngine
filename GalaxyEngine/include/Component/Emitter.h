#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"

namespace GALAXY 
{
    namespace Resource {class Sound;}
    
    enum class AttenuationModel
    {
        None,
        Inverse,
        Linear,
        Exponential
    };

    inline const char* SerializeAttenuationModelEnum()
    {
        return "None\0Inverse\0Linear\0Exponential\0";
    }
    
    namespace Component
    {
        class GALAXY_API Emitter : public IComponent<Emitter>
        {
        public:
            Emitter() = default;
            Emitter& operator=(const Emitter& other) = default;
            Emitter(const Emitter&) = default;
            Emitter(Emitter&&) noexcept = default;
            ~Emitter() override = default;
            
            const char* GetComponentName() const override { return "Emitter"; }

            void ShowInInspector() override;
            void OnTransformUpdate();

            void OnCreate() override;
            void OnDestroy() override;

            void Play();
            void Stop();

            Shared<Resource::Sound> GetSound() const { return m_sound.lock(); }
            float GetVolume() const { return m_volume; }
            float GetPitch() const { return m_pitch; }
            float GetDuration() const { return m_duration; }
            float GetMinDistance() const { return m_minDistance; }
            float GetMaxDistance() const { return m_maxDistance; }
            AttenuationModel GetAttenuationModel() const { return m_attenuationModel; }
            bool IsLooping() const { return m_isLooping; }
            float GetDopplerFactor() const { return m_dopplerFactor; }
            float GetPan() const { return m_pan; }

            void SetSound(Shared<Resource::Sound> sound);
            void SetVolume(float volume);
            void SetPitch(float pitch);
            void SetMinDistance(float minDistance);
            void SetMaxDistance(float maxDistance);
            void SetAttenuationModel(AttenuationModel attenuationModel);
            void SetLooping(bool isLooping);
            void SetDopplerFactor(float dopplerFactor);
            void SetPan(float pan);
            
            void Serialize(CppSer::Serializer& serializer) override;
            void Deserialize(CppSer::Parser& parser) override;
            void AfterLoad() override;

        private:
            void SetPosition(const Vec3f& position);
            void SetDirection(const Vec3f& direction);

        private:
            Weak<Resource::Sound> m_sound;
            float m_volume = 0.1f;
            float m_duration = 0.0f;
            float m_minDistance = 0.0f;
            float m_maxDistance = 100.0f;
            float m_pitch = 1.0f;
            float m_pan = 0.0f;
            bool m_isLooping = false;
            float m_dopplerFactor = 1.0f;

            AttenuationModel m_attenuationModel = AttenuationModel::Inverse;
        };
        
        
    }
    
}
