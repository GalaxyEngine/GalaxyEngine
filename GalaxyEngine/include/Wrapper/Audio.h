#pragma once
#include "GalaxyAPI.h"

#include <memory>
#include <filesystem>

#include "Core/GameObject.h"
#include "Core/UUID.h"

struct ma_device;
struct ma_context;
struct ma_sound;
struct ma_engine;
namespace GALAXY 
{
	enum class AttenuationModel;

	namespace Component
	{
		class Emitter;
		class Listener;
	}

	namespace Resource
	{
		class Sound;
	}
	namespace Wrapper 
	{		
		class Audio
		{
		public:
			Audio() {}
			Audio& operator=(const Audio& other) = default;
			Audio(const Audio&) = default;
			Audio(Audio&&) noexcept = default;
			virtual ~Audio() {}

			bool Initialize();

			void Release();

			static Audio* GetInstance() {
				if (s_instance == nullptr) {
					s_instance = std::make_unique<Audio>();
				}
				return s_instance.get();
			}

			bool LoadSound(const std::filesystem::path& fullPath, const Core::UUID& uuid);
			void UnloadSound(const Core::UUID& uuid);

			void AddListener(Component::Listener* listener);
			void AddEmitter(const Component::Emitter* emitter);

			void RemoveListener(const Component::Listener* listener);
			void RemoveEmitter(const Component::Emitter* emitter);

			void UpdateEmitterSound(const Component::Emitter* emitter);

			void SetEmitterPosition(const Core::UUID& uuid, const Vec3f& position);
			void SetEmitterDirection(const Core::UUID& uuid, const Vec3f& direction);

			void SetListenerPosition(const uint32_t& id, const Vec3f& position);
			void SetListenerDirection(const uint32_t& id, const Vec3f& direction);

			void Play_Sound(Resource::Sound* sound);
			void Stop_Sound();

			void PlayEmitter(const Core::UUID& uuid);
			void StopEmitter(const Core::UUID& uuid);
			void SetEmitterVolume(const Core::UUID& uuid, float volume);
			void SetMinDistance(const Core::UUID& uuid, float minDistance);
			void SetMaxDistance(const Core::UUID& uuid, float maxDistance);
			void SetPitch(const Core::UUID& uuid, float pitch);
			void SetAttenuationModel(const Core::UUID& uuid, AttenuationModel model);
			void SetPan(const Core::UUID& uuid, float pan);
			void SetDopplerFactor(const Core::UUID& uuid, float factor);
			void SetLooping(const Core::UUID& uuid, bool loop);
			bool IsPlaying(const Core::UUID& uuid);

			static uint32_t AttenuationModelToAPI(AttenuationModel model);

			void SetPlaybackTime(const Core::UUID& uuid, float time);
			
			float GetPlaybackTime(const Core::UUID& uuid);
			float GetSoundDuration(const Core::UUID& uuid);
		private:
			ma_sound* GetSoundFromUUID(const Core::UUID& uuid);
		private:
			static std::unique_ptr<Audio> s_instance;

			ma_engine* m_engine = nullptr;

			std::unordered_map<Core::UUID, ma_sound*> m_loadedSounds;

			std::set<uint32_t> m_listeners;
		};
	}
}
