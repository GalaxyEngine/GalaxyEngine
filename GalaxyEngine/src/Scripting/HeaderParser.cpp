#include "pch.h"
#include "Scripting/HeaderParser.h"

namespace GALAXY 
{
	Scripting::HeaderParser::HeaderParser()
	{

	}

	std::vector<GALAXY::Scripting::PropertyInfo> Scripting::HeaderParser::ParseFile(const std::filesystem::path& filename)
	{
		std::ifstream file(filename);
		std::vector<PropertyInfo> properties;

		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		std::string buffer(size, ' ');
		file.seekg(0);
		file.read(&buffer[0], size);
		std::vector<size_t> Ppos;

		const char* searchWord = "UPROPERTY";
		size_t pos = buffer.find(searchWord);

		std::string content = buffer;
		while (pos != std::string::npos) {
			pos = content.find(searchWord);
			if (pos == std::string::npos)
				break;
			content = content.substr(pos);
			std::string line = content.substr(0, content.find_first_of('\n'));
			PropertyInfo property;
			if (ExtractPropertyInfo(line, property, buffer))
			{
				properties.push_back(property);
			}
			content = content.substr(line.length());
		}

		return properties;
	}

	bool Scripting::HeaderParser::ExtractPropertyInfo(const std::string& line, PropertyInfo& propertyInfo, const std::string& file)
	{
		// Get Variable Name
		std::string variableName = line.substr(line.find_last_of(',') + 1);
		while (!isValidVariableCharacter(variableName[0]))
		{
			variableName.erase(variableName.begin());
		}

		while (!isValidVariableCharacter(variableName.back()))
		{
			variableName.pop_back();
		}

		propertyInfo.propertyName = variableName;

		// Get Variable Type
		if (auto pos = file.find(variableName); pos != std::string::npos)
		{
			size_t newPos = pos;
			while (file[newPos] != '\n')
			{
				newPos--;
			}
			std::string lineVariable = file.substr(newPos + 1);
			lineVariable = lineVariable.substr(0, lineVariable.find_first_of('\n'));


			while (!isValidVariableCharacter(lineVariable[0]))
			{
				lineVariable.erase(lineVariable.begin());
			}

			if (pos = lineVariable.find("class"); pos != std::string::npos)
			{
				lineVariable = lineVariable.substr(6);
			}

			lineVariable = lineVariable.substr(0, lineVariable.find_first_of(' '));
			propertyInfo.propertyType = lineVariable;
			return true;
		}

		return false;
	}

	bool Scripting::HeaderParser::isValidVariableCharacter(char ch)
	{
		// Check if the character is an uppercase letter, lowercase letter, digit, or underscore
		return std::isalpha(ch) || std::isdigit(ch) || ch == '_';
	}

}
