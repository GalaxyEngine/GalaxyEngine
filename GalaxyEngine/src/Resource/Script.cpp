#include "pch.h"
#include "Resource/Script.h"
#include "Resource/ResourceManager.h"

#include "Scripting/ScriptEngine.h"
namespace GALAXY
{
	const char* hFileContent = R"(
	#include "Component/IComponent.h"
	#include "Scripting/Macro.h"
	#include "Component/ScriptComponent.h"
	
	class %s : public Component::ScriptComponent
	{
	    GENERATED_BODY(%s)
	public:
	};
	UCLASS(%s)
	)";

	void Resource::Script::Load()
	{
		if (p_loaded)
			return;
		std::weak_ptr<Script> resource = Resource::ResourceManager::GetInstance()->GetResource<Script>(this->GetFileInfo().GetRelativePath());
		Scripting::ScriptEngine::GetInstance()->AddScript(resource);
		p_loaded = true;
	}

	Weak<Resource::Script> Resource::Script::Create(const std::filesystem::path& path)
	{
		std::ofstream hFile(path.string() + ".h");
		auto className = path.filename();
		char content[512];
		snprintf(content, sizeof(content), hFileContent, className.string().c_str(), className.string().c_str(), className.string().c_str());
		if (hFile.is_open()) {

			hFile << content;
			hFile.close();
		}

		std::ofstream cppFile(path.string() + ".cpp");
		if (cppFile.is_open()) {
			std::string cppContent;
			cppContent = std::string("#include \"") + path.filename().string() + ".h\"";
			cppFile << cppContent;
			cppFile.close();
		}

		return Resource::ResourceManager::GetInstance()->GetOrLoad<Script>(path.string() + ".h");
	}

}
