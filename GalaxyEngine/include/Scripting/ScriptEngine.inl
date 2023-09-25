#pragma once
#include "Scripting/ScriptEngine.h"
namespace GALAXY 
{
	Scripting::ScriptEngine* Scripting::ScriptEngine::GetInstance()
	{
		if (!m_instance)
			m_instance = std::make_unique<ScriptEngine>();
		return m_instance.get();
	}
}
