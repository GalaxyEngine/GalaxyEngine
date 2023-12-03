#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
namespace GALAXY 
{
	namespace Scripting
	{
		struct HFile
		{
			explicit HFile(const std::filesystem::path& _path) : path(_path) {}
			std::filesystem::path path;
		};

		struct PropertyInfo {
			std::string propertyName;
			std::string propertyType;
			bool isAList = false;
		};

		class HeaderParser
		{
		public:
			HeaderParser();

			List<PropertyInfo> ParseFile(const std::filesystem::path& filename);
			static bool ExtractPropertyInfo(const std::string& line, PropertyInfo& propertyInfo, std::string file);

			static bool isValidVariableCharacter(char ch);
		private:
		};
	}
}
#include "Scripting/HeaderParser.inl" 
;