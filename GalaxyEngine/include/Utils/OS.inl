#pragma once
#include <regex>

#include "Utils/OS.h"

#ifdef __linux__
#include <dlfcn.h>
#endif

namespace GALAXY 
{
#ifdef _WIN32

struct VSAppInfo {
    std::string displayName;
    std::string version;
    std::string productPath;
    std::string installationPath;  // Example of an extra field
};

using VSAppInfos = std::vector<VSAppInfo>;

inline VSAppInfos ParseVSWhereResult(const std::string& input) {
    VSAppInfos apps;
    
    // Fields to search for
    std::vector<std::string> fields = { "installationVersion", "productPath",
    	"displayName", "installationPath" };

    // Regex pattern for key-value pairs
    std::regex fieldRe(R"((\w+):\s*([^\r\n]+))");
    
    // Find each instance block by detecting "instanceId:" as a separator
    std::regex instanceRe(R"(instanceId:\s*[^\r\n]+)");
    auto instanceStart = std::sregex_iterator(input.begin(), input.end(), instanceRe);
    auto instanceEnd = std::sregex_iterator();

    for (auto it = instanceStart; it != instanceEnd; ++it) {
        // Get the substring for this instance
        auto startPos = it->position();
        auto nextPos = (std::next(it) != instanceEnd) ? std::next(it)->position() : input.length();
        std::string instanceBlock = input.substr(startPos, nextPos - startPos);
        
        // Extract fields
        std::unordered_map<std::string, std::string> fieldValues;
        std::sregex_iterator fieldStart(instanceBlock.begin(), instanceBlock.end(), fieldRe);
        std::sregex_iterator fieldEnd;
        
        for (auto fit = fieldStart; fit != fieldEnd; ++fit) {
            fieldValues[(*fit)[1].str()] = (*fit)[2].str();
        }

        // Ensure required fields exist before adding to the result
        if (fieldValues.count("displayName") && fieldValues.count("installationVersion") && fieldValues.count("productPath")) {
            apps.emplace_back(VSAppInfo{
                fieldValues["displayName"],
                fieldValues["installationVersion"],
                fieldValues["productPath"],
                fieldValues.count("installationPath") ? fieldValues["installationPath"] : ""  // Optional field
            });
        }
    }
    return apps;
}


	inline std::filesystem::path Utils::FindTool::FindVS()
	{
		std::filesystem::path result;

		std::filesystem::path vsPath = OS::GetEnvVar("VSINSTALLDIR");
		if (vsPath.empty())
		{
			vsPath = OS::GetEnvVar("ProgramFiles(x86)");
			if (vsPath.empty())
			{
				vsPath = OS::GetEnvVar("ProgramFiles");
			}
			if (!vsPath.empty())
			{
				vsPath /= "Microsoft Visual Studio\\Installer";
			}
		}
		
		if (vsPath.empty())
		{
			return result;
		}

		std::string command = "cd \"" + vsPath.string() + "\" && " + "vswhere.exe";
		std::string output = OS::RunCommand(command, false);

		VSAppInfos apps = ParseVSWhereResult(output);

		//Choose the latest version of Visual Studio
		std::sort(apps.begin(), apps.end(), [](const VSAppInfo& a, const VSAppInfo& b) {
			return a.version > b.version;
		});

		if (apps.size() > 0)
		{
			result = apps[0].productPath;
		}
		
		return result;
	}
#endif

	template<typename T>
	inline T Utils::OS::GetDLLMethod(void* dll, const char* name)
	{
#ifdef _WIN32
		return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(dll), name));
#elif defined(__linux__)
		return reinterpret_cast<T>(dlsym(dll, name));
#endif
		return T();
	}
	
	inline std::filesystem::path Utils::OS::GetEnvVar(std::string const& key)
	{
#ifdef _WIN32
		size_t requiredSize = 0;
		// Get the size of the environment variable's value.
		errno_t err = getenv_s(&requiredSize, nullptr, 0, key.c_str());
		if (err != 0 || requiredSize == 0)
		{
			return std::string("");
		}

		// Allocate a buffer of the appropriate size.
		std::vector<char> buffer(requiredSize);
		err = getenv_s(&requiredSize, buffer.data(), requiredSize, key.c_str());
		if (err != 0)
		{
			return std::string("");
		}

		return std::string(buffer.data());
#else
		char * val = getenv(key.c_str());
		return val == nullptr ? std::string("") : std::string(val);
#endif
	}

}
