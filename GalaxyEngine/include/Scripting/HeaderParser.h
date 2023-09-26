#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Scripting
	{
		struct HFile
		{
			HFile(const std::filesystem::path& _path) : path(_path) {}
			std::filesystem::path path;
		};

		struct PropertyInfo {
			std::string propertyName;
			std::string propertyType;
		};

		class HeaderParser
		{
		public:
			HeaderParser();

			void FindFiles(const std::filesystem::path& path);

			std::vector<PropertyInfo> ParseFile(const std::string& filename);
			bool ExtractPropertyInfo(const std::string& line, PropertyInfo& propertyInfo, const std::string& file);

		private:
			std::vector<HFile> hFiles;
		};
	}
}
#include "Scripting/HeaderParser.inl" 
