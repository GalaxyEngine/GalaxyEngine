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
	{
		auto projectPath = Resource::ResourceManager::GetInstance()->GetProjectPath();
		m_scriptEngine = GS::ScriptEngine::Get();
		m_scriptEngine->SetCopyToFolder(std::filesystem::current_path() / "ProjectsDLL");
		m_scriptEngine->SetHeaderGenFolder(projectPath / "Generate" / "Headers");
	}

	Scripting::ScriptEngine::~ScriptEngine()
	{
	}

	void Scripting::ScriptEngine::RegisterScriptComponents()
	{
		for (auto& instance : m_scriptEngine->GetAllScriptInstances())
		{
			auto scriptComp = static_cast<Component::ScriptComponent*>(instance.second->m_constructor());
			scriptComp->InitializeVariablesInfo();
			Component::ComponentHolder::RegisterComponent(scriptComp);
		}
	}

	void Scripting::ScriptEngine::UnregisterScriptComponents()
	{
		for (auto& instance : m_scriptEngine->GetAllScriptInstances())
		{
			Component::ComponentHolder::UnregisterComponentByName(instance.first.c_str());
		}
	}

	void Scripting::ScriptEngine::UpdateFileWatch()
	{
		m_currentTime += Wrapper::GUI::DeltaTime();
		if (m_currentTime > m_updateInterval)
		{
			std::filesystem::path dllPathExt = m_dllPath.string() + Utils::OS::GetDLLExtension();
			if (!std::filesystem::exists(dllPathExt))
				return;
			std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(dllPathExt);
			if (m_lastWriteTime < lastWriteTime || !m_lastWriteTime.has_value())
			{
				PrintLog("Reloading DLL: %s", dllPathExt.string());
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

	void Scripting::ScriptEngine::FreeDLL()
	{
		if (!m_scriptEngine)
			return;
		UnregisterScriptComponents();
		m_scriptEngine->FreeDLL();

	}

	void Scripting::ScriptEngine::LoadDLL(const std::filesystem::path& dllPath)
	{
		m_dllPath = dllPath;
		if (!m_scriptEngine->LoadDLL(dllPath))
		{
			PrintError("Failed to load DLL: %s", dllPath.string().c_str());
			return;
		}
		m_lastWriteTime = std::filesystem::last_write_time(m_dllPath.string() + Utils::OS::GetDLLExtension());
	}


	void Scripting::ScriptEngine::ReloadDLL()
	{
		std::string content;
		if (m_dllPath.empty())
			return;

		auto rootGameObject = Core::SceneHolder::GetCurrentScene()->GetRootGameObject();

		CppSer::Serializer serializer;
		for (auto& child : rootGameObject.lock()->GetAllChildren())
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
		content = serializer.GetContent();

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

			const Shared<Component::BaseComponent> script = instanceScriptComponent->Clone();

			script->Deserialize(parser);

			uint32_t componentID = parser["EDITOR ComponentID"].As<uint32_t>();
			uint64_t entityID = parser["EDITOR EntityID"].As<uint64_t>();
			auto gameObject = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithUUID(entityID);

			assert(gameObject.lock() != nullptr);

			gameObject.lock()->AddComponent(script, componentID);

			parser.PushDepth();
		} while (parser.GetCurrentDepth() < parser.GetValueMap().size());

	}

	void* Scripting::ScriptEngine::GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName)
	{
		return m_scriptEngine->GetScriptVariable(scriptComponent, scriptName, variableName);
	}

	void Scripting::ScriptEngine::SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, void* value)
	{
		m_scriptEngine->SetScriptVariable(scriptComponent, scriptName, variableName, value);
	}

#ifdef WITH_EDITOR
	void Scripting::ScriptEngine::CompileCode()
	{
		const Path prevPath = std::filesystem::current_path();
		const Path projectPath = Resource::ResourceManager::GetProjectPath();

		// Execute your build commands
		auto threadMethod = [&]()
		{
			std::filesystem::current_path(projectPath);
#ifdef _MSC_VER
			Utils::OS::RunCommand("xmake f -p windows -a x64 -m debug");
#elif defined(__linux__)
			Utils::OS::RunCommand("xmake f -p linux -a x64 -m debug");
#endif
			Utils::OS::RunCommand("xmake");
			std::filesystem::current_path(prevPath);
		};
		Core::ThreadManager::GetInstance()->AddTask(threadMethod);
	}

	void Scripting::ScriptEngine::GenerateSolution(Editor::ScriptEditorTool tool)
	{
		const Path prevPath = std::filesystem::current_path();
		const Path projectPath = Resource::ResourceManager::GetProjectPath();
		std::filesystem::current_path(projectPath);
		switch (tool)
		{
#ifdef _WIN32
		case Editor::ScriptEditorTool::Rider:
		case Editor::ScriptEditorTool::VisualStudio:
		{
				auto threadMethod = [&](){
					Utils::OS::RunCommand("xmake f -p windows -a x64 -m debug");
					Utils::OS::RunCommand("xmake project -k vsxmake");
				};
				Core::ThreadManager::GetInstance()->AddTask(threadMethod);
			break;
		}
#endif
		case Editor::ScriptEditorTool::VisualStudioCode:
		{
				auto threadMethod = [&]()
				{
					Utils::OS::RunCommand("xmake project -k compile_commands .vscode");
					std::ofstream file(".vscode/c_cpp_properties.json");
					if (file.is_open()) {
						file << std::string(R"(
{
   "configurations": [
       {
           "compileCommands": ".vscode/compile_commands.json"
       }
   ],
   "version": 4
})");
					}
				};
				Core::ThreadManager::GetInstance()->AddTask(threadMethod);
			break;
		}
		default:
			PrintError("Unsupported script editor tool: %s", Editor::SerializeScriptEditorToolValue(tool));
			break;
		}
		std::filesystem::current_path(prevPath);
	}

	void Scripting::ScriptEngine::OpenSolution(Editor::ScriptEditorTool tool)
	{
		switch (tool)
		{
#ifdef _WIN32
		case Editor::ScriptEditorTool::VisualStudio:
		{
			Utils::OS::OpenWithVS(Resource::ResourceManager::GetProjectPath().string() + "\"");
			break;
		}
		case Editor::ScriptEditorTool::Rider:
		{
			Utils::OS::OpenWithRider(Resource::ResourceManager::GetProjectPath().string() + "\"");
			break;
		}
#endif
		case Editor::ScriptEditorTool::VisualStudioCode:
		{
			Utils::OS::OpenWithVSCode(Resource::ResourceManager::GetProjectPath().string() + "\"");
			break;
		}
		case Editor::ScriptEditorTool::Custom:
		{
			auto editorSettings = Core::Application::GetInstance().GetEditorSettings();
			Utils::OS::OpenWith(editorSettings.GetOtherScriptEditorToolPath(),Resource::ResourceManager::GetProjectPath().string() + "\"");
			break;
		}
		}
	}
#endif

	std::unordered_map<std::string, std::shared_ptr<Scripting::VariableInfo>> Scripting::ScriptEngine::GetAllScriptVariablesInfo(const std::string& scriptName)
	{
		std::unordered_map<std::string, std::shared_ptr<Scripting::VariableInfo>> variables;
		for (auto& variable : m_scriptEngine->GetAllScriptVariablesInfo(scriptName))
		{
			VariableType variableType = VariableType::Unknown;
			auto variableTypeName = variable.second.property.type;

			VariableInfo::SanitizeType(variableTypeName);

			variableType = VariableInfo::TypeNameToType(variableTypeName);

			switch (variableType)
			{
			case Scripting::VariableType::None:
				variables[variable.first] = std::make_shared<VariableInfo>(variable.second.property);
				break;
			case Scripting::VariableType::Unknown:
				variables[variable.first] = std::make_shared<VariableInfo>(variable.second.property);
				break;
			case Scripting::VariableType::Bool:
				variables[variable.first] = std::make_shared<VariableInfoT<bool>>(variable.second.property);
				break;
			case Scripting::VariableType::Int:
				variables[variable.first] = std::make_shared<VariableInfoT<int>>(variable.second.property);
				break;
			case Scripting::VariableType::Float:
				variables[variable.first] = std::make_shared<VariableInfoT<float>>(variable.second.property);
				break;
			case Scripting::VariableType::Double:
				variables[variable.first] = std::make_shared<VariableInfoT<double>>(variable.second.property);
				break;
			case Scripting::VariableType::String:
				variables[variable.first] = std::make_shared<VariableInfoT<std::string>>(variable.second.property);
				break;
			case Scripting::VariableType::Vector2f:
				variables[variable.first] = std::make_shared<VariableInfoT<Vec2f>>(variable.second.property);
				break;
			case Scripting::VariableType::Vector3f:
				variables[variable.first] = std::make_shared<VariableInfoT<Vec3f>>(variable.second.property);
				break;
			case Scripting::VariableType::Vector4f:
				variables[variable.first] = std::make_shared<VariableInfoT<Vec4f>>(variable.second.property);
				break;
			case Scripting::VariableType::Quaternion:
				variables[variable.first] = std::make_shared<VariableInfoT<Quat>>(variable.second.property);
				break;
			case Scripting::VariableType::GameObject:
				variables[variable.first] = std::make_shared<VariableInfoT<Core::GameObject*>>(variable.second.property);
				break;
			case Scripting::VariableType::Component:
				variables[variable.first] = std::make_shared<VariableInfoT<Component::BaseComponent*>>(variable.second.property);
			default:
				break;
			}
		}
		return variables;
	}

}
