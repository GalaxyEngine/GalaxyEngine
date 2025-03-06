#include "pch.h"
#include "Scripting/ScriptEngine.h"

#include <galaxyscript/ScriptEngine.h>
#include <fstream>

#include "Core/SceneHolder.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Component/ComponentHolder.h"
#include "Component/ScriptComponent.h"

#include "Utils/OS.h"

#ifdef WITH_EDITOR
#include "Editor/EditorSettings.h"
#endif
namespace GALAXY
{
	std::unique_ptr<Scripting::ScriptEngine> Scripting::ScriptEngine::s_instance;

	Scripting::ScriptEngine::ScriptEngine()
	= default;

	Scripting::ScriptEngine::~ScriptEngine()
	= default;

	void Scripting::ScriptEngine::Initialize()
	{
		Path projectPath = Resource::ResourceManager::GetProjectPath();
		m_engine = GS::ScriptEngine::Get();
#ifdef WITH_EDITOR
		m_dllCopyToFolder = std::filesystem::current_path() / "ProjectsDLL";
		m_engine->SetCopyToFolder(m_dllCopyToFolder);
#else
		m_engine->SetCopyToFolder("");
#endif
		m_engine->SetHeaderGenFolder(projectPath / "Generate" / "Headers");
		
		if (!Resource::ResourceManager::DoesProjectExists())
			return;
		
		String projectName = Resource::ResourceManager::GetProjectName();
		Path exePath = Core::Application::GetExePath();

		Path dllPath = projectPath / "Generate" / projectName;

		Path dllFullPath = dllPath.string().append(DLL_EXT);
		Path galaxyDll = Utils::FileSystem::FindFileWithNameInFolder(exePath.parent_path(), "GalaxyEngine" DLL_EXT, true, false);
#ifdef WITH_GAME
		if (!std::filesystem::exists(dllPath.generic_string() + DLL_EXT))
		{
			dllPath = projectPath / (PACKAGE_ASSEMBLY_NAME);
			dllFullPath = dllPath.string().append(DLL_EXT);
		}
		
		if (!galaxyDll.empty() && std::filesystem::exists(dllFullPath))
		{
			auto engineLastModif = std::filesystem::last_write_time(galaxyDll);
			auto dllLastModif = std::filesystem::last_write_time(dllFullPath);
#ifdef WITH_PACKAGE
			ASSERT(engineLastModif < dllLastModif && "Project DLL is older than the engine dll")
#else
			PrintError("Project DLL is older than the engine DLL");
#endif
		}
		
		LoadDLL(dllPath.generic_string().c_str());
#else
		bool shouldCompile = false;
		if (!galaxyDll.empty() && std::filesystem::exists(dllFullPath))
		{
			auto engineLastModif = std::filesystem::last_write_time(galaxyDll);
			auto dllLastModif = std::filesystem::last_write_time(dllFullPath);
			shouldCompile = dllLastModif < engineLastModif;
			SetDLLPath(dllPath);
		}
		bool loaded = false;
		if (!shouldCompile)
		{
			loaded = LoadDLL(dllPath.generic_string().c_str());
		}
		else
		{
			DeleteProjectDLL();
		}
		if (!loaded)
		{
			ResetLastWriteTime();
			CompileCode(shouldCompile);
		}
#endif
	}

	void Scripting::ScriptEngine::RegisterScriptComponents() const
	{
		for (auto& instance : m_engine->GetAllScriptInstances())
		{
			auto scriptComp = static_cast<Component::ScriptComponent*>(instance.second->m_constructor());
			scriptComp->InitializeVariablesInfo();
			Component::ComponentHolder::RegisterComponent(scriptComp);
		}
	}

	void Scripting::ScriptEngine::UnregisterScriptComponents()
	{
		for (auto& instance : m_engine->GetAllScriptInstances())
		{
			Component::ComponentHolder::UnregisterComponentByName(instance.first);
		}
	}

	void Scripting::ScriptEngine::UpdateFileWatch()
	{
		m_currentTime += Wrapper::GUI::DeltaTime();
		if (m_currentTime > m_updateInterval)
		{
			std::filesystem::path dllPathExt = m_dllPath.string() + Utils::OS::GetDLLExtension();
			if (!std::filesystem::exists(dllPathExt) || Utils::OS::IsFileLocked(dllPathExt))
				return;
			std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(dllPathExt);
			if (m_lastWriteTime < lastWriteTime || !m_lastWriteTime.has_value())
			{
				PrintLog("Reloading DLL: %s", dllPathExt.string().c_str());
				ReloadDLL();
			}
			m_currentTime = 0;
		}
	}

	Scripting::ScriptEngine* Scripting::ScriptEngine::GetInstance()
	{
		if (s_instance == nullptr)
		{
			s_instance = std::make_unique<ScriptEngine>();
		}
		return s_instance.get();
	}

	void Scripting::ScriptEngine::ResetLastWriteTime()
	{
		
		m_lastWriteTime.reset();
	}

	std::filesystem::path Scripting::ScriptEngine::GetFilePathForScript(const std::string& scriptClassName)
	{
		Path projectPath = Resource::ResourceManager::GetProjectPath();
		Path genPathFolder = projectPath / "Generate" / "Headers";

		auto parseGenFile = [&](const std::filesystem::path& headerPath)
		{
			CppSer::Parser parser(headerPath);

			do
			{
				const std::string className = parser["Class Name"];

				if (className == scriptClassName)
					return true;
				parser.PushDepth();
			}
			while (parser.GetValueMap().size() > parser.GetCurrentDepth());

			return false;
		};

		std::filesystem::path genFileName = "";
		std::vector<std::filesystem::path> genFiles;
		for (const auto& entry : std::filesystem::directory_iterator(genPathFolder))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".gen")
			{
				if (entry.path().filename().string() == scriptClassName)
				{
					if (parseGenFile(entry.path()))
					{
						genFileName = entry.path().filename().stem();
					}
				}
				else
				{
					genFiles.push_back(entry.path());
				}
			}
		}

		if (genFileName.empty())
		{
			for (const auto& genFile : genFiles)
			{
				if (parseGenFile(genFile))
					genFileName = genFile.filename().stem();
			}
		}

		if (genFileName.empty())
			return {};

		genFileName = genFileName.generic_string() + ".h";

		auto assetPath = Resource::ResourceManager::GetAssetPath();
		return Utils::FileSystem::FindFileWithNameInFolder(assetPath, genFileName.generic_string(), true, true);
	}

	void Scripting::ScriptEngine::FreeDLL()
	{
		if (!m_engine)
			return;
		UnregisterScriptComponents();
		m_engine->FreeDLL();

	}

	bool Scripting::ScriptEngine::LoadDLL(const std::filesystem::path& dllPath)
	{
		m_dllPath = dllPath;
#ifdef WITH_EDITOR
		if (!m_engine->LoadDLL(dllPath))
#else
		if (!m_engine->LoadDLL(dllPath, false))
#endif
		{
			// Can happen if the dll is not found, or the dll is not valid (compiled with another compiler)
			PrintError("Failed to load DLL: %s", dllPath.string().c_str());
			m_lastWriteTime = std::filesystem::file_time_type::max(); // max value to not spam the reloadDll method
			return false;
		}
		else
		{
			PrintLog("Loaded DLL %s successfully", dllPath.string().c_str());
		}
		m_lastWriteTime = std::filesystem::last_write_time(m_dllPath.string() + Utils::OS::GetDLLExtension());
		return true;
	}


	void Scripting::ScriptEngine::ReloadDLL()
	{
		if (m_dllPath.empty())
			return;

		auto rootGameObject = Core::SceneHolder::GetCurrentScene()->GetRootGameObject();

		CppSer::Serializer serializer;
		for (auto& child : rootGameObject->GetAllChildren())
		{
			for (auto& scriptComponent : child.lock()->GetComponents<Component::ScriptComponent>())
			{
				// Serialize
				serializer << CppSer::Pair::BeginMap << "ScriptComponent";
				serializer << CppSer::Pair::Key << "EDITOR ScriptName" << CppSer::Pair::Value << scriptComponent.lock()->GetComponentName();
				serializer << CppSer::Pair::Key << "EDITOR ComponentID" << CppSer::Pair::Value << scriptComponent.lock()->GetIndex();
				serializer << CppSer::Pair::Key << "EDITOR EntityID" << CppSer::Pair::Value << child.lock()->GetUUID();
				scriptComponent.lock()->Serialize(serializer);
				serializer << CppSer::Pair::EndMap;

				child.lock()->RemoveComponent(scriptComponent.lock().get());
			}
		}
		std::string content = serializer.GetContent();

		FreeDLL();
		LoadDLL(m_dllPath);
		RegisterScriptComponents();

		CppSer::Parser parser(content);
		do
		{
			std::string scriptName = parser["EDITOR ScriptName"].As<std::string>();
			Shared<Component::BaseComponent> instanceScriptComponent = nullptr;
			for (const auto& instanceComponent : Component::ComponentHolder::GetList())
			{
				if (instanceComponent->GetComponentName() == scriptName)
				{
					instanceScriptComponent = instanceComponent;
					break;
				}
			}

			if (!instanceScriptComponent)
			{
				parser.PushDepth();
				continue;
			}


			uint32_t componentID = parser["EDITOR ComponentID"].As<uint32_t>();
			uint64_t entityID = parser["EDITOR EntityID"].As<uint64_t>();
			
			auto gameObject = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithUUID(entityID);
			const Shared<Component::BaseComponent> script = instanceScriptComponent->Clone();
			// Setup manually
			std::dynamic_pointer_cast<Component::ScriptComponent>(script)->SetupVariables();

			script->SetGameObject(gameObject.lock().get());
			script->Deserialize(parser);

			assert(gameObject.lock() != nullptr);

			gameObject.lock()->AddComponent(script, componentID);

			parser.PushDepth();
		} while (parser.GetCurrentDepth() < parser.GetValueMap().size());

	}

	void Scripting::ScriptEngine::DeleteProjectDLL() const
	{
		if (m_dllPath.empty() || m_dllCopyToFolder.empty())
			return;

		auto file = m_dllPath.generic_string().append(DLL_EXT);
		if (file.empty())
			return;
		std::remove(file.c_str());
	}

	void* Scripting::ScriptEngine::GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName) const
	{
		return m_engine->GetScriptVariable(scriptComponent, scriptName, variableName);
	}

	void Scripting::ScriptEngine::SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, void* value) const
	{
		m_engine->SetScriptVariable(scriptComponent, scriptName, variableName, value);
	}

#ifdef WITH_EDITOR
	void Scripting::ScriptEngine::CompileCode(bool force /*= false*/, bool monothread /*= false*/)
	{
		const Path projectPath = Resource::ResourceManager::GetProjectPath();

		ASSERT(projectPath.empty() == false && "Project path is empty");
		PrintLog("Compiling project %s", projectPath.generic_string().c_str());
		
		std::string mode = "debug";
#ifdef NDEBUG
		mode = "release";
#endif
		
		std::string platformSpecific;
		switch (Editor::GetUserCompiler()) {
		case Editor::CompilerTool::MSVC:
			platformSpecific = "xmake f -p windows -a x64 -m " + mode;
			break;
		case Editor::CompilerTool::GCC:
			platformSpecific = "xmake f -p linux -a x64 -m " + mode;
			break;
		case Editor::CompilerTool::MINGW:
			platformSpecific = "xmake f -p mingw -a x64 -m " + mode;
			break;
		default:
			ASSERT(false && "Compiler tool not supported");
			break;
		}
		std::string command = "cd " + projectPath.generic_string()
		+ " && " + platformSpecific;
		if (force)
			command += " && xmake clean";
		command += " && xmake";
		if (monothread)
			Utils::OS::RunCommand(command);
		else
			Utils::OS::RunCommandThread(command);
	}

	void Scripting::ScriptEngine::GenerateSolution(Editor::ScriptEditorToolType tool)
	{
		const Path projectPath = Resource::ResourceManager::GetProjectPath();
		switch (tool)
		{
#ifdef _WIN32
		case Editor::ScriptEditorToolType::Rider:
		case Editor::ScriptEditorToolType::VisualStudio:
		{
				std::string command = "cd " + projectPath.generic_string()
					+ " && xmake f -p windows -a x64 -m debug"
					+ " && xmake project -k vsxmake";
				Utils::OS::RunCommandThread(command);
			break;
		}
#endif
		case Editor::ScriptEditorToolType::VisualStudioCode:
		{
				auto threadMethod = [projectPath]()
				{
					std::string command = "cd " + projectPath.generic_string()
						+ " && xmake project -k compile_commands .vscode";
					Utils::OS::RunCommand(command);
					std::ofstream file(projectPath / ".vscode/c_cpp_properties.json");
					if (file.is_open()) {
						std::string c_cpp_properties("{\n\t\"configurations\": [\n\t\t {\n\t\t\t\"compileCommands\":\".vscode/compile_commands.json\",\n\t\t\t\"includePath\": [\"%s\"]\n\t\t }\n\t],\n\t\"version\": 4\n}");
						auto includePath = std::filesystem::current_path().parent_path() / "GalaxyEngine/include";
						c_cpp_properties = Debug::FormatString(c_cpp_properties.c_str(), includePath.generic_string().c_str());
						file << c_cpp_properties;
					}
				};
				Core::ThreadManager::GetInstance()->AddTask(threadMethod);
			break;
		}
		default:
			PrintError("Unsupported script editor tool: %s", Editor::to_string(tool));
			break;
		}
	}

#ifdef _WIN32
	std::filesystem::path Scripting::ScriptEngine::GetSLNPath()
	{
		return Resource::ResourceManager::GetAssetPath().parent_path() / "vsxmake2022" / (
			Resource::ResourceManager::GetProjectPath().filename().stem().string() + ".sln");
	}
#endif

	void Scripting::ScriptEngine::OpenSolution(Editor::ScriptEditorToolType tool)
	{
		Path path = Resource::ResourceManager::GetProjectPath().string() + "\"";
		
		Editor::EditorSettings editorSettings = Core::Application::GetInstance().GetEditorSettings();
		Path currentEditorToolPath = editorSettings.GetCurrentScriptEditorToolPath();
		switch (tool)
		{
#ifdef _WIN32
		case Editor::ScriptEditorToolType::VisualStudio:
			{
				Utils::OS::OpenWithVS(path);
				break;
			}
		case Editor::ScriptEditorToolType::Rider:
			{
				Utils::OS::OpenWithRider("");
				break;
			}
#endif
		case Editor::ScriptEditorToolType::VisualStudioCode:
			{
				Utils::OS::OpenWithVSCode(path);
				break;
			}
		case Editor::ScriptEditorToolType::Custom:
			{
				Utils::OS::OpenWith(currentEditorToolPath, path);
				break;
			}
		case Editor::ScriptEditorToolType::None:
			{
				PrintError("No script editor tool selected");
			}
			break;
		default:;
		}
	}
#endif

	std::unordered_map<std::string, std::shared_ptr<Scripting::VariableInfo>> Scripting::ScriptEngine::GetAllScriptVariablesInfo(const std::string& scriptName) const
	{
		std::unordered_map<std::string, std::shared_ptr<VariableInfo>> variables;
		for (auto& variable : m_engine->GetAllScriptVariablesInfo(scriptName))
		{
			VariableType variableType = VariableType::Unknown;
			auto variableTypeName = variable.second.property.type;

			VariableInfo::SanitizeType(variableTypeName);

			variableType = VariableInfo::TypeNameToType(variableTypeName);

			switch (variableType)
			{
			case VariableType::None:
			case VariableType::Unknown:
				variables[variable.first] = std::make_shared<VariableInfo>(variable.second.property);
				break;
			case VariableType::Bool:
				variables[variable.first] = std::make_shared<VariableInfoT<bool>>(variable.second.property);
				break;
			case VariableType::Int:
				variables[variable.first] = std::make_shared<VariableInfoT<int>>(variable.second.property);
				break;
			case VariableType::Float:
				variables[variable.first] = std::make_shared<VariableInfoT<float>>(variable.second.property);
				break;
			case VariableType::Double:
				variables[variable.first] = std::make_shared<VariableInfoT<double>>(variable.second.property);
				break;
			case VariableType::String:
				variables[variable.first] = std::make_shared<VariableInfoT<std::string>>(variable.second.property);
				break;
			case VariableType::Vector2f:
				variables[variable.first] = std::make_shared<VariableInfoT<Vec2f>>(variable.second.property);
				break;
			case VariableType::Vector3f:
				variables[variable.first] = std::make_shared<VariableInfoT<Vec3f>>(variable.second.property);
				break;
			case VariableType::Vector4f:
				variables[variable.first] = std::make_shared<VariableInfoT<Vec4f>>(variable.second.property);
				break;
			case VariableType::Quaternion:
				variables[variable.first] = std::make_shared<VariableInfoT<Quat>>(variable.second.property);
				break;
			case VariableType::GameObject:
				variables[variable.first] = std::make_shared<VariableInfoT<Core::GameObject*>>(variable.second.property);
				break;
			case VariableType::Component:
				variables[variable.first] = std::make_shared<VariableInfoT<Component::BaseComponent*>>(variable.second.property);
			default:
				break;
			}
		}
		return variables;
	}

}
