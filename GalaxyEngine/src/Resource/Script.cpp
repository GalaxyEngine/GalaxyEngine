#include "pch.h"
#include "Resource/Script.h"
#include "Resource/ResourceManager.h"

#ifdef WITH_EDITOR
#include "Editor/EditorSettings.h"
#endif

#include "ScriptEngine.h"
#include "Scripting/ScriptEngine.h"

#include "Utils/OS.h"

namespace GALAXY
{
	const char* hFileContent =
		R"(
#pragma once
#include "Scripting/Macro.h"
#include "Component/ScriptComponent.h"
#include "%s.generated.h"
// Note : all ptr variable need to be initialized with nullptr

GCLASS()
class %s : public Component::ScriptComponent
{
	GENERATED_BODY()
public:

public:
	void OnStart() override
	{

	}

	void OnUpdate() override
	{

	}

};

END_FILE()
	)";

	void Resource::Script::Load()
	{
		// do not load .cpp file
		if (p_loaded || this->p_fileInfo.GetExtension() == ".cpp")
			return;
		std::weak_ptr<Script> resource = Resource::ResourceManager::GetInstance()->GetResource<Script>(this->GetFileInfo().GetRelativePath());
		//GS::ScriptEngine::Get()->AddHeader(resource);
		p_loaded = true;

		//OpenScript(GetFileInfo().GetFullPath());
	}

	void Resource::Script::Unload()
	{
		// Remove the resource from ScriptEngine, remove all component when the user rebuild the dll
		const std::weak_ptr<Script> resource = Resource::ResourceManager::GetInstance()->GetResource<Script>(this->GetFileInfo().GetRelativePath());
		//GS::ScriptEngine::Get()->RemoveScript(resource);
		p_loaded = false;
	}

	Weak<Resource::Script> Resource::Script::Create(const Path& path)
	{
		std::ofstream hFile(path.string() + ".h");
		auto className = path.filename();
		char content[512];
		snprintf(content, sizeof(content), hFileContent, className.string().c_str(), className.string().c_str());
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

		Resource::ResourceManager::GetInstance()->GetOrLoad<Script>(path.string() + ".cpp");
		auto scriptHeader = Resource::ResourceManager::GetInstance()->GetOrLoad<Script>(path.string() + ".h");
		Scripting::ScriptEngine::CompileCode();
		return scriptHeader;
	}

#ifdef WITH_EDITOR
	void Resource::Script::OpenScript(const Path& path)
	{
		Editor::ScriptEditorTool tool = Editor::EditorSettings::GetInstance().GetScriptEditorTool();
		switch (tool)
		{
#ifdef _WIN32
		case Editor::ScriptEditorTool::VisualStudio:
		{
			OpenWithVS(path);
			break;
		}
#endif
		case Editor::ScriptEditorTool::VisualStudioCode:
		{
			OpenWithVSCode(path);
			break;
		}
		default:
			break;
		}
	}

	void Resource::Script::OpenWithVSCode(const Path& path)
	{
		Utils::OS::OpenWithVSCode(ResourceManager::GetInstance()->GetAssetPath().parent_path().string() + "\"");
	}

	void Resource::Script::OpenWithVS(const Path& path)
	{
		Utils::OS::OpenWithVS(path);
	}
#endif
}
