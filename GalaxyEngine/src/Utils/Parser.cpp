#include "pch.h"
#include "Utils/Parser.h"
namespace GALAXY
{
#pragma region Serializer
	Utils::Serializer::Serializer(const std::filesystem::path& path)
	{
		Create(path);
	}

	Utils::Serializer::~Serializer()
	{
		CloseFile();
	}

	void Utils::Serializer::Create(const std::filesystem::path& path)
	{
		m_file = std::ofstream(path);
		if (!m_file.is_open()) {
			PrintError("File %s could'nt be create", path.string().c_str());
			m_file.close();
		}
	}

	void Utils::Serializer::CloseFile()
	{
		m_file.close();
	}

	template<typename T> Utils::Serializer& Utils::Serializer::operator<<(const T& value)
	{
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const char& value)
	{
		std::string stringValue(1, value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const std::filesystem::path& value)
	{
		*this << value.string().c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const bool& value)
	{
		std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const float& value)
	{
		std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const int& value)
	{
		std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const double& value)
	{
		std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Vec2f& value)
	{
		std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Vec3f& value)
	{
		std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Vec4f& value)
	{
		std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Quat& value)
	{
		std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Mat4& value)
	{
		std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Utils::PAIR& value)
	{
		SetCurrentType(value);
		return *this;
	}

	Utils::Serializer& Utils::Serializer::operator<<(const char* value)
	{
		switch (m_currentType)
		{
		case Utils::PAIR::KEY:
		{
			SetCurrentKey(value);
		}
		break;
		case Utils::PAIR::VALUE:
		{
			SetCurrentValue(value);
			WriteLine();
		}
		break;
		default:
			break;
		}
		return *this;
	}

	void Utils::Serializer::SetCurrentType(Utils::PAIR val)
	{
		switch (val)
		{
		case PAIR::KEY:
			m_currentType = val;
			break;
		case  PAIR::VALUE:
			m_currentType = val;
			break;
		case PAIR::BEGIN_TAB:
			PushTab();
			break;
		case PAIR::END_TAB:
			PopTab();
			break;
		default:
			break;
		}
	}

	void Utils::Serializer::WriteLine()
	{
		AddLine(m_currentPair.first, m_currentPair.second);
	}
#pragma endregion

#pragma region Parser


	Utils::Parser::Parser(const std::filesystem::path& path)
	{
		Parse(path);
	}

	void Utils::Parser::Parse(const std::filesystem::path& path)
	{
		m_file = std::fstream(path, std::ios::in);
		if (!m_file.is_open()) {
			PrintError("File %s could'nt be open", path.string().c_str());
			m_file.close();
			return;
		}

		std::string line;
		size_t currentDepth = 0;
		std::unordered_map<std::string, std::string>* currentMap = nullptr;

		m_valueMap.push_back({});
		currentMap = &m_valueMap.back();

		while (std::getline(m_file, line)) {
			size_t depth = std::count(line.begin(), line.end(), '\t');
			if (depth != currentDepth)
			{
				currentDepth = depth;
				m_valueMap.push_back({});
				currentMap = &m_valueMap.back();
			}

			if (auto pos = line.find("["); pos != std::string::npos)
			{
				std::string currentKey = line.substr(pos + 1);
				currentKey = currentKey.substr(0, currentKey.find_first_of(']'));
				std::string currentValue = line.substr(line.find_first_of(':') + 2);

				currentMap->insert({ currentKey, currentValue });
			}
		}

		m_file.close();
	}

	void Utils::Parser::PrintData()
	{
		for (auto& maps : m_valueMap)
		{
			for (auto& [key, value] : maps)
			{
				std::cout << key << " : " << value << std::endl;
			}
			std::cout << "---------------------------" << std::endl;
		}
	}

	void Utils::Parser::NewDepth()
	{
		m_currentDepth++;
	}

	Utils::StringSerializer Utils::Parser::operator[](const std::string& key)
	{
		ASSERT(m_valueMap.size() > m_currentDepth);
		ASSERT(m_valueMap[m_currentDepth].count(key) > 0);
		return StringSerializer(m_valueMap[m_currentDepth][key]);
	}

#pragma endregion

	template <typename T>
	T Utils::StringSerializer::As()
	{
		return static_cast<T>(m_content);
	}

	template <>
	float Utils::StringSerializer::As()
	{
		return std::stof(m_content);
	}

	template <>
	double Utils::StringSerializer::As()
	{
		return std::stod(m_content);
	}

	template <>
	int Utils::StringSerializer::As()
	{
		return std::stoi(m_content);
	}

	template <>
	bool Utils::StringSerializer::As()
	{
		return std::stoi(m_content);
	}

	template <>
	Vec2f Utils::StringSerializer::As()
	{
		return Vec2f(m_content);
	}

	template <>
	Vec3f Utils::StringSerializer::As()
	{
		return Vec3f(m_content);
	}

	template <>
	Vec4f Utils::StringSerializer::As()
	{
		return Vec4f(m_content);
	}

	template <>
	Quat Utils::StringSerializer::As()
	{
		return Quat(m_content);
	}
}
