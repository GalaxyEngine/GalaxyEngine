#pragma once
#include <filesystem>

#include "GalaxyAPI.h"

namespace GALAXY 
{
	namespace Resource
	{
		class Texture;
	}
	namespace Render
	{
		class Framebuffer;
	}
	namespace Utils::OS
	{
		struct Filter
		{
			Filter(std::string _name, std::string _spec) : name(std::move(_name)), spec(std::move(_spec)) {}
			
			std::string name;
			// ex : "Text file"
			std::string spec;
			// ex : "txt"
		};

		GALAXY_API std::filesystem::path GetUserAppDataFolder();
		
		std::string SaveDialog(const std::vector<Filter>& filters);

		std::string OpenDialog(const std::vector<Filter>& filters);

		// Show or Hide file with the path of the file
		void ShowFile(const std::filesystem::path& filePath, bool showFile = true);

		std::string GetLastErrorMessage();

		void* LoadDLL(const std::filesystem::path& DllPath);

		void FreeDLL(void* dll);

		template<typename T>
		inline T GetDLLMethod(void* dll, const char* name);

		void ShowInExplorer(const std::filesystem::path& folder, const std::vector<std::string>& filesName);

		const char* GetDLLExtension();

		void OpenWithVSCode(const std::filesystem::path& filePath);

		void OpenWith(const std::filesystem::path& appPath, const std::filesystem::path& filePath);
#ifdef _WIN32
		void OpenWithVS(const std::filesystem::path& filePath);
		void OpenWithRider(const std::filesystem::path& filePath);
#endif
		void RunCommand(const std::string& command);
		void RunCommandThread(const std::string& command);

		void DisplayImageInPopup(const std::filesystem::path& imagePath, int windowWidth = 400, int windowHeight = 400);
		void DisplayImageInPopup(Render::Framebuffer* framebuffer, int windowWidth = 400, int windowHeight = 400);
		void DisplayImageInPopup(Resource::Texture* texture, int windowWidth = 400, int windowHeight = 400);
		void DisplayImageInPopup(const Wrapper::Image& image, int windowWidth = 400, int windowHeight = 400);
	}
}
#include "Utils/OS.inl" 
