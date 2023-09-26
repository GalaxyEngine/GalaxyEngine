#pragma once
#include "GalaxyAPI.h"
#include <unordered_map>
namespace GALAXY 
{
	namespace Scripting {
		class ScriptEngine
		{
		public:
			ScriptEngine();
			~ScriptEngine();

			static inline ScriptEngine* GetInstance();

			void LoadDLL(const std::filesystem::path& dllPath, const std::string& dllName);

			void ReloadDLL();
		private:

		private:
			static std::unique_ptr<ScriptEngine> m_instance;

			HMODULE m_hDll = NULL;
			bool m_dllLoaded = false;

			std::filesystem::path m_dllPath;
			std::string m_dllName;

			std::unordered_map<std::string, std::shared_ptr<class ScriptInstance>> m_scriptInstances;
			std::vector<std::weak_ptr<class ScriptComponent>> m_scriptComponents;
		};
	}
}
#include "Scripting/ScriptEngine.inl" 
