#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
#include <functional>
#include <unordered_map>

#include <Wrapper/GUI.h>

namespace GS { class ScriptEngine; struct Property; }
namespace GALAXY
{
	namespace Scripting
	{


		enum class VariableType
		{
			None,
			Unknown,
			Bool,
			Int,
			Float,
			Double,
			String,
			Vector2f,
			Vector3f,
			Vector4f,
			Quaternion,
		};

		struct VariableInfo
		{
			VariableInfo() = default;
			VariableInfo(const GS::Property& variable);

			std::vector<std::string> args;
			std::string name;
			std::string typeName;

			VariableType type = VariableType::None;
			bool isAList = false;
			std::function<void(const std::string& name, void* value)> displayValue = nullptr;
			std::function<void(const std::string& name, void* value)> displayValue = nullptr;
		private:
			template<typename T>
			inline void ManageValue()
			{
				if (isAList)
				{
					this->displayValue = [&](const std::string& name, void* value) {
						std::vector<T>* list = (std::vector<T>*)value;
						if (!list)
							return

							Wrapper::GUI::SetNextItemOpen();
						if (Wrapper::GUI::TreeNode(name.c_str())) {
							for (size_t i = 0; i < list->size(); i++) {
								Wrapper::GUI::PushID(static_cast<int>(i));
								T valueList = (*list)[i];

								DisplayValue<T>(name, &valueList);
								(*list)[i] = valueList;
								Wrapper::GUI::PopID();
							}
							if (Wrapper::GUI::Button("+")) {
								list->push_back({});
							}
							Wrapper::GUI::SameLine();
							if (Wrapper::GUI::Button("-") && !list->empty()) {
								list->pop_back();
							}
							Wrapper::GUI::TreePop();
						}
						};
				}
				else
				{
					auto boundFunction = std::bind(
						&Scripting::VariableInfo::DisplayValue<T>,
						this,
						std::placeholders::_1,
						std::placeholders::_2
					);

					displayValue = boundFunction;
				}
			}

			void SetDisplayValue();

			template<typename T>
			void DisplayValue(const std::string& name, void* value);

		};

		class ScriptEngine
		{
		public:
			ScriptEngine();
			void RegisterScriptComponents();
			void UnregisterScriptComponents();

			void UpdateFileWatch() {} // TODO

			void LoadDLL(const std::filesystem::path& dllPath);

			void ReloadDLL(); //TODO : Do all the work of hot reload

			void* GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName);
			void SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, void* value);

			template<typename T>
			T* GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName)
			{
				return reinterpret_cast<T*>(GetScriptVariable(scriptComponent, scriptName, variableName));
			}

			template<typename T>
			void SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, T* value)
			{
				SetScriptVariable(scriptComponent, scriptName, variableName, reinterpret_cast<void*>(value));
			}

			std::unordered_map<std::string, Scripting::VariableInfo> GetAllScriptVariablesInfo(const std::string& scriptName);

			static ScriptEngine* GetInstance();
		private:
			static std::unique_ptr<ScriptEngine> s_instance;

			GS::ScriptEngine* m_scriptEngine = nullptr;

			std::filesystem::path m_dllPath;
		};
	}
}