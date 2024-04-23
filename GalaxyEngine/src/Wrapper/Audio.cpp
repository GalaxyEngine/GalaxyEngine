#include "pch.h"
#include "Wrapper/Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "Component/Emitter.h"
#include "Component/Listener.h"
#include "Component/Transform.h"
#include "Core/GameObject.h"
#include "Resource/Sound.h"


namespace GALAXY
{
	std::unique_ptr<Wrapper::Audio> Wrapper::Audio::s_instance;

	void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
	{
		// In playback mode copy data to pOutput. In capture mode read data from pInput. In full-duplex mode, both
		// pOutput and pInput will be valid and you can move data from pInput into pOutput. Never process more than
		// frameCount frames.
	}

	bool Wrapper::Audio::Initialize()
	{
		m_engine = new ma_engine();
		auto result = ma_engine_init(NULL, m_engine);
		if (result != MA_SUCCESS) {
			PrintError("Failed to initialize miniaudio engine");
			return false;
		}

		//For the sound resource : https://github.com/mackron/miniaudio/issues/249
		//	- Set a different ma_sound for each emitter no matter if the sound is loaded or not
		//  - So for the resource, we only need to now his path.
		//  - release the ma_sound with ma_sound_uninit

		//TODO : Handle enable/disable of components and gameObjects
		
		PrintLog("Audio system initialized with miniaudio");
		return true;
	}

	void Wrapper::Audio::Release()
	{

		for (auto& sound : m_loadedSounds) {
			ma_sound_uninit(sound.second);
			delete sound.second;
		}
		m_loadedSounds.clear();
		
		ma_engine_uninit(m_engine);
		delete m_engine;
		m_engine = nullptr;
	}


	bool Wrapper::Audio::LoadSound(const std::filesystem::path& fullPath, const Core::UUID& uuid)
	{
		auto sound = new ma_sound();
		m_loadedSounds[uuid] = sound;

		ma_result result = ma_sound_init_from_file(m_engine, fullPath.string().c_str(), 0, NULL, NULL, sound);
		if (result != MA_SUCCESS) {
			PrintError("Failed to load sound");
			return false;
		}
		return true;
	}

	void Wrapper::Audio::UnloadSound(const Core::UUID& uuid)
	{
		if (m_loadedSounds.find(uuid) == m_loadedSounds.end()) {
			PrintError("Sound not found");
			return;
		}
		ma_sound* s = m_loadedSounds[uuid];
		m_loadedSounds.erase(uuid);
		ma_sound_uninit(s);
		delete s;
	}

	void Wrapper::Audio::AddListener(Component::Listener* listener)
	{
		uint32_t index = 0;
		while (m_listeners.find(index) != m_listeners.end()) {
			++index;
		}
		m_listeners.insert(index);
		listener->m_listenerIndex = index;
		
		ma_engine_listener_set_enabled(m_engine, index, listener->IsEnable());

		Vec3f worldPosition = listener->GetTransform()->GetWorldPosition();
		Vec3f direction = listener->GetTransform()->GetForward();
		ma_engine_listener_set_position(m_engine, index, worldPosition.x, worldPosition.y, worldPosition.z);
		ma_engine_listener_set_direction(m_engine, index, direction.x, direction.y, direction.z);
		ma_engine_listener_set_world_up(m_engine, index, 0.0f, 1.0f, 0.0f);
	}

	void Wrapper::Audio::AddEmitter(const Component::Emitter* emitter)
	{
		if (emitter->GetSound() != nullptr)
			UpdateEmitterSound(emitter);
	}

	void Wrapper::Audio::RemoveListener(const Component::Listener* listener)
	{
		m_listeners.erase(listener->m_listenerIndex);
		ma_engine_listener_set_enabled(m_engine, listener->m_listenerIndex, false);
		ma_engine_listener_set_position(m_engine, listener->m_listenerIndex, 0.0f, 0.0f, 0.0f);
		ma_engine_listener_set_direction(m_engine, listener->m_listenerIndex, 0.0f, 0.0f, 0.0f);
	}

	void Wrapper::Audio::RemoveEmitter(const Component::Emitter* emitter)
	{
		UnloadSound(emitter->GetGameObject()->GetUUID());
	}

	void Wrapper::Audio::UpdateEmitterSound(const Component::Emitter* emitter)
	{
		if (emitter->GetSound() == nullptr)
			return;
		
		Core::GameObject* gameObject = emitter->GetGameObject();
		auto uuid = gameObject->GetUUID();
		const Vec3f worldPosition = gameObject->GetTransform()->GetWorldPosition();
		const Vec3f direction = gameObject->GetTransform()->GetForward();
		
		//TODO Use other UUID
		if (m_loadedSounds.find(uuid) != m_loadedSounds.end())
			UnloadSound(uuid);
		
		LoadSound(emitter->GetSound()->GetFileInfo().GetFullPath(), uuid);
		ma_sound* s = m_loadedSounds[uuid];

		SetEmitterPosition(uuid, worldPosition);
		SetEmitterDirection(uuid, direction);
		SetEmitterVolume(uuid, emitter->GetVolume());
		SetLooping(uuid, emitter->IsLooping());
		SetDopplerFactor(uuid, emitter->GetDopplerFactor());
		SetAttenuationModel(uuid, emitter->GetAttenuationModel());
		SetPan(uuid, emitter->GetPan());
		SetPitch(uuid, emitter->GetPitch());
	}

	void Wrapper::Audio::SetEmitterPosition(const Core::UUID& uuid, const Vec3f& position)
	{
		if (m_loadedSounds.find(uuid) == m_loadedSounds.end()) {
			PrintError("Sound not found");
			return;
		}
		ma_sound* s = m_loadedSounds[uuid];
		ma_sound_set_position(s, position.x, position.y, position.z);
	}

	void Wrapper::Audio::SetEmitterDirection(const Core::UUID& uuid, const Vec3f& direction)
	{
		if (m_loadedSounds.find(uuid) == m_loadedSounds.end()) {
			PrintError("Sound not found");
			return;
		}
		ma_sound* s = m_loadedSounds[uuid];
		ma_sound_set_direction(s, direction.x, direction.y, direction.z);
	}

	void Wrapper::Audio::SetListenerPosition(const uint32_t& id, const Vec3f& position)
	{
		ma_engine_listener_set_position(m_engine, id, position.x, position.y, position.z);
	}

	void Wrapper::Audio::SetListenerDirection(const uint32_t& id, const Vec3f& direction)
	{
		ma_engine_listener_set_direction(m_engine, id, direction.x, direction.y, direction.z);
	}

	void Wrapper::Audio::Play_Sound(Resource::Sound* sound)
	{
		ma_engine_set_volume(m_engine, 0.1f);
		ma_engine_play_sound(m_engine, sound->GetFileInfo().GetFullPath().string().c_str(), nullptr);
	}

	void Wrapper::Audio::Stop_Sound()
	{
		ma_engine_uninit(m_engine);
		ma_engine_init(nullptr, m_engine);
	}

	void Wrapper::Audio::PlayEmitter(const Core::UUID& uuid)
	{		
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		
		auto result = ma_sound_start(s);
		if (result != MA_SUCCESS) {
			PrintError("Failed to start sound");
			return;
		}
	}

	void Wrapper::Audio::StopEmitter(const Core::UUID& uuid)
	{		
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		
		auto result = ma_sound_stop(s);
		if (result != MA_SUCCESS) {
			PrintError("Failed to stop sound");
			return;
		}
	}

	void Wrapper::Audio::SetEmitterVolume(const Core::UUID& uuid, float volume)
	{		
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		ma_sound_set_volume(s, volume);
	}

	void Wrapper::Audio::SetMinDistance(const Core::UUID& uuid, float minDistance)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		ma_sound_set_min_distance(s, minDistance);
	}

	void Wrapper::Audio::SetMaxDistance(const Core::UUID& uuid, float maxDistance)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		ma_sound_set_max_distance(s, maxDistance);
	}

	void Wrapper::Audio::SetPitch(const Core::UUID& uuid, float pitch)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		ma_sound_set_pitch(s, pitch);
		ma_sound_set_attenuation_model(s, ma_attenuation_model_exponential);
	}

	void Wrapper::Audio::SetAttenuationModel(const Core::UUID& uuid, AttenuationModel model)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		auto attenuationModel = static_cast<ma_attenuation_model>(AttenuationModelToAPI(model));
		ma_sound_set_attenuation_model(s, attenuationModel);
	}

	void Wrapper::Audio::SetPan(const Core::UUID& uuid, float pan)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		ma_sound_set_pan(s, pan);
	}

	void Wrapper::Audio::SetDopplerFactor(const Core::UUID& uuid, float factor)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		ma_sound_set_doppler_factor(s, factor);
	}

	void Wrapper::Audio::SetLooping(const Core::UUID& uuid, bool loop)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		ma_sound_set_looping(s, loop);
	}

	bool Wrapper::Audio::IsPlaying(const Core::UUID& uuid)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return false;
		return ma_sound_is_playing(s);
	}

	uint32_t Wrapper::Audio::AttenuationModelToAPI(AttenuationModel model)
	{
		switch (model) {
		case AttenuationModel::None:
			return ma_attenuation_model_none;
		case AttenuationModel::Inverse:
			return ma_attenuation_model_inverse;
		case AttenuationModel::Linear:
			return ma_attenuation_model_linear;
		case AttenuationModel::Exponential:
			return ma_attenuation_model_exponential;
		default:
			PrintError("Invalid model");
			return ma_attenuation_model_none;
		}
	}

	void Wrapper::Audio::SetPlaybackTime(const Core::UUID& uuid, float time)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return;
		uint32_t sampleRate = ma_engine_get_sample_rate(m_engine);
		auto source = ma_sound_get_data_source(s);
		auto result = ma_data_source_seek_to_pcm_frame(source, static_cast<ma_uint64>(sampleRate * time));
		if (result != MA_SUCCESS) {
			PrintError("Failed to seek");
			return;
		}
	}

	float Wrapper::Audio::GetPlaybackTime(const Core::UUID& uuid)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return 0.0f;
		
		auto source = ma_sound_get_data_source(s);
		float cursor;
		auto result = ma_data_source_get_cursor_in_seconds(source, &cursor);
		if (result != MA_SUCCESS) {
			PrintError("Failed to get cursor");
			return 0.0f;
		}
		return cursor;
	}

	float Wrapper::Audio::GetSoundDuration(const Core::UUID& uuid)
	{
		ma_sound* s = GetSoundFromUUID(uuid);
		if (s == nullptr)
			return 0.0f;
		
		auto source = ma_sound_get_data_source(s);
		float duration;
		auto result = ma_data_source_get_length_in_seconds(source, &duration);
		if (result != MA_SUCCESS) {
			PrintError("Failed to get duration");
			return 0.0f;
		}
		return duration;
	}

	ma_sound* Wrapper::Audio::GetSoundFromUUID(const Core::UUID& uuid)
	{
		if (m_loadedSounds.find(uuid) == m_loadedSounds.end()) {
			PrintError("Sound not found");
			return nullptr;
		}
		return m_loadedSounds[uuid];
	}
}
