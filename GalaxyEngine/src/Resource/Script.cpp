#include "pch.h"
#include "Resource/Script.h"
#include "Resource/ResourceManager.h"

#include "Scripting/ScriptEngine.h"
namespace GALAXY 
{
	void Resource::Script::Load()
	{
		if (p_loaded)
			return;
		std::weak_ptr<Script> resource = Resource::ResourceManager::GetInstance()->GetResource<Script>(this->GetFileInfo().GetRelativePath());
		Scripting::ScriptEngine::GetInstance()->AddScript(resource);
		p_loaded = true;
	}
}
