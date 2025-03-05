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
		if (p_loaded)
			return;
		p_loaded = true;
		
#ifdef WITH_EDITOR
		m_scriptContent = Utils::FileSystem::ReadFile(GetFileInfo().GetFullPath());
#endif
	}

	void Resource::Script::Unload()
	{
		p_loaded = false;
	}

#ifdef WITH_EDITOR
	void Resource::Script::ShowInInspector()
	{
		IResource::ShowInInspector();

		ImGui::SeparatorText("Content");
		ImGui::TextWrapped(m_scriptContent.c_str());
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

		ResourceManager::GetOrLoad<Script>(path.string() + ".cpp");
		auto scriptHeader = ResourceManager::GetOrLoad<Script>(path.string() + ".h");
		Scripting::ScriptEngine::CompileCode();
		return scriptHeader;
	}

	void Resource::Script::OpenScript(const Path& path)
	{
		const Editor::ScriptEditorToolType tool = Editor::EditorSettings::GetInstance().GetScriptEditorToolType();
		switch (tool)
		{
#ifdef _WIN32
		case Editor::ScriptEditorToolType::VisualStudio:
		{
			OpenWithVS(path);
			break;
		}
		case Editor::ScriptEditorToolType::Rider:
		{
			OpenWithRider(path);
			break;
		}
#endif
		case Editor::ScriptEditorToolType::VisualStudioCode:
		{
			OpenWithVSCode(path);
			break;
		}
		case Editor::ScriptEditorToolType::None:
		default:
			break;
		}
	}

	void Resource::Script::OpenWithVSCode(const Path& path)
	{
		Utils::OS::OpenWithVSCode(ResourceManager::GetProjectPath().generic_string() + "\" " + path.generic_string() + "\"");
	}
#ifdef _WIN32
	void Resource::Script::OpenWithVS(const Path& path)
	{
		Utils::OS::OpenWithVS(path);
	}
	void Resource::Script::OpenWithRider(const Path& path)
	{
		Utils::OS::OpenWithRider(path);
	}
#endif
#endif
}
