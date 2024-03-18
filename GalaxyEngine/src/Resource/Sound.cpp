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
		auto audioInstance = Wrapper::Audio::GetInstance();
		//audioInstance->
	}

	void Resource::Sound::Unload()
	{

	}

}
