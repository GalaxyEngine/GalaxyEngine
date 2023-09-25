#include "pch.h"
#include "Scripting/ScriptEngine.h"
#define DESTINATION_DLL std::filesystem::path("")

namespace GALAXY 
{
	std::unique_ptr<Scripting::ScriptEngine> Scripting::ScriptEngine::m_instance;
	Scripting::ScriptEngine::ScriptEngine()
	{

	}

	Scripting::ScriptEngine::~ScriptEngine()
	{
	}

	void Scripting::ScriptEngine::LoadDLL(const std::filesystem::path& dllPath, const std::string& dllName)
	{
		auto dllPathName = dllPath / (dllName + ".dll");
		auto pdbPathName = dllPath / (dllName + ".pdb");

		if (!std::filesystem::exists(dllPathName))
		{
			PrintError("Failed to load project DLL.");
			return;
		}

		m_dllName = dllName;

		std::filesystem::path copiedDllPath = DESTINATION_DLL / (dllName + ".dll");
		std::filesystem::path copiedPdbPath = DESTINATION_DLL / (dllName + ".pdb");

		Utils::FileSystem::RemoveFile(copiedDllPath);
		Utils::FileSystem::RemoveFile(copiedPdbPath);

		Utils::FileSystem::CopyFileTo(dllPathName, copiedDllPath);
		Utils::FileSystem::CopyFileTo(pdbPathName, copiedPdbPath);

		m_hDll = LoadLibrary(copiedDllPath.string().c_str());
		if (m_hDll != NULL) {
			PrintLog("Loading Project %s", dllName.c_str());
		}
		else {
			PrintError("Failed to load project DLL.");
		}
	}

	void Scripting::ScriptEngine::ReloadDLL()
	{
		if (m_dllLoaded && m_hDll != NULL)
		{
			FreeLibrary(m_hDll);
		}
	}

}
