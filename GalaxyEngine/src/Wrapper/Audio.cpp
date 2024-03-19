#include "pch.h"
#include "Wrapper/Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

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
		ma_result result;

		ma_context_config contextConfig = ma_context_config_init();

		m_device = new ma_device();
		m_context = new ma_context();

		ma_context_config context_config = ma_context_config_init();

		result = ma_context_init(NULL, 0, &context_config, m_context);
		if (result != MA_SUCCESS) {
			PrintError("Failed to initialize miniaudio context");
				return false;
		}

		ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format = ma_format_f32;
		deviceConfig.playback.channels = 2;
		deviceConfig.sampleRate = 44100;
		deviceConfig.dataCallback = data_callback;
		deviceConfig.pUserData = this;


		result = ma_device_init(m_context, &deviceConfig, m_device);
		if (result != MA_SUCCESS) {
			PrintError("Failed to initialize miniaudio playback device");
			ma_context_uninit(m_context);
			return false;
		}

		result = ma_device_start(m_device);
		if (result != MA_SUCCESS) {
			PrintError("Failed to start miniaudio playback device");
			ma_device_uninit(m_device);
			ma_context_uninit(m_context);
			return false;
		}

		PrintLog("Audio system initialized with miniaudio");
		return true;
	}

	void Wrapper::Audio::Release()
	{
		if (m_device) 
		{
			ma_device_uninit(m_device);
			delete m_device;
		}
		if (m_context) 
		{
			ma_context_uninit(m_context);
			delete m_context;
		}
	}


	bool Wrapper::Audio::LoadSound(const std::filesystem::path& fullPath, Resource::Sound* outputSound)
	{
		ma_decoder decoder;
		ma_result result = ma_decoder_init_file(fullPath.string().c_str(), NULL, &decoder);
		if (result != MA_SUCCESS) {
			return false;
		}


		ma_decoder_uninit(&decoder);
	}

}
