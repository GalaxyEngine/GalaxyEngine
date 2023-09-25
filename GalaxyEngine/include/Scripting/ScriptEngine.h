#pragma once
#include "GalaxyAPI.h"
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
		};
	}
}
#include "Scripting/ScriptEngine.inl" 
