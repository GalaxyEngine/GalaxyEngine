#include "pch.h"
#include "Resource/Sound.h"

#include "Wrapper/Audio.h"

namespace GALAXY 
{

	Resource::Sound::Sound(const Path& fullPath) : IResource(fullPath)
	{

	}

	void Resource::Sound::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		p_loaded = true;

		CreateDataFile();
		SendRequest();
	}

	void Resource::Sound::Unload()
	{
		auto audioInstance = Wrapper::Audio::GetInstance();
		// audioInstance->UnloadSound(this);
	}

	void Resource::Sound::Play()
	{
		auto audioInstance = Wrapper::Audio::GetInstance();
		audioInstance->Play_Sound(this);
	}

	void Resource::Sound::Stop()
	{
		auto audioInstance = Wrapper::Audio::GetInstance();
		audioInstance->Stop_Sound();
	}

	void Resource::Sound::ShowInInspector()
	{
#ifdef WITH_EDITOR
		IResource::ShowInInspector();

		
		if (ImGui::Button("Play"))
		{
			Play();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			Stop();
		}
#endif
	}

}
