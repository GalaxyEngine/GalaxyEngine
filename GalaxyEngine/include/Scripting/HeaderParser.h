#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
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

			std::vector<PropertyInfo> ParseFile(const std::filesystem::path& filename);
			bool ExtractPropertyInfo(const std::string& line, PropertyInfo& propertyInfo, const std::string& file);

			bool isValidVariableCharacter(char ch);
		private:
		};
	}
}
#include "Scripting/HeaderParser.inl" 
;