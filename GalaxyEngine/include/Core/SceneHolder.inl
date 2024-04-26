#pragma once
#include "Core/SceneHolder.h"
namespace GALAXY 
{
	inline void Core::SceneHolder::SwitchScene(const Weak<Resource::Scene>& scene, bool copyData/* = false*/)
	{
		m_nextScene = scene.lock();

#if WITH_EDITOR
		m_copyData = copyData;
#endif
	}
}
