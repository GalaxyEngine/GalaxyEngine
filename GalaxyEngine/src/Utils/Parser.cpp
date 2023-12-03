#include "pch.h"
#include "Utils/Parser.h"
#include "Core/GameObject.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

#define MAP_SEPARATOR_BEGIN " ------------- "
#define MAP_SEPARATOR_END " ============= "

namespace GALAXY
{
#pragma region Serializer
	Utils::Serializer::Serializer(const std::filesystem::path& path)
	{
		m_filePath = path;
	}

	Utils::Serializer::~Serializer()
	{
		if (!m_filePath.empty())
			CloseFile();
	}

	void Utils::Serializer::Create(const std::filesystem::path& path)
	{
		m_filePath = path;
	}

	void Utils::Serializer::CloseFile() const
	{
		auto file = std::ofstream(m_filePath);
		if (!file.is_open()) {
			PrintError("File %s could'nt be create", m_filePath.string().c_str());
			file.close();
		}
		file << m_content.str();
		file.close();
	}

	template<typename T> Utils::Serializer& Utils::Serializer::operator<<(const T& value)
	{
		return *this;
	}

	template<typename T> Utils::Serializer& Utils::Serializer::operator<<(T* value)
	{
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const std::string& value)
	{
		*this << value.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const char& value)
	{
		const std::string stringValue(1, value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const std::filesystem::path& value)
	{
		*this << value.generic_string().c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const bool& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const float& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const int& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const unsigned long& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}
	template<> Utils::Serializer& Utils::Serializer::operator<<(const long long& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const unsigned long long& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Core::UUID& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const double& value)
	{
		const std::string stringValue = std::to_string(value);
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Vec2f& value)
	{
		const std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Vec3f& value)
	{
		const std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Vec4f& value)
	{
		const std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Quat& value)
	{
		const std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Mat4& value)
	{
		const std::string stringValue = value.ToString();
		*this << stringValue.c_str();
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(const Utils::Pair& value)
	{
		SetCurrentType(value);
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(Core::GameObject* value)
	{
		if (!value) {
			this->operator<<(-1);
			return *this;
		}
		;
		this->operator<<<uint64_t>(value->GetUUID());
		return *this;
	}

	template<> Utils::Serializer& Utils::Serializer::operator<<(Component::BaseComponent* value)
	{
		std::string pairString;
		if (!value)
			pairString = std::to_string(-1) + ", " + std::to_string(-1);
		else
			pairString = std::to_string(value->GetGameObject()->GetUUID()) + ", " + std::to_string(value->GetIndex());
		this->operator<<(pairString);
		return *this;
	}


	Utils::Serializer& Utils::Serializer::operator<<(const char* value)
	{
		switch (m_currentType)
		{
		case Utils::Pair::KEY:
		{
			SetCurrentKey(value);
		}
		break;
		case Utils::Pair::VALUE:
		{
			SetCurrentValue(value);
			WriteLine();
		}
		break;
		case Utils::Pair::TITLE:
		{
			AddLine(value);
		}
		break;
		case Utils::Pair::BEGIN_MAP:
		{
			m_content << value << MAP_SEPARATOR_BEGIN << '\n';
		}
		break;
		case Utils::Pair::END_MAP:
		{
			m_content << value << MAP_SEPARATOR_END << '\n';
		}
		break;
		default:
			break;
		}
		return *this;
	}

	void Utils::Serializer::SetCurrentType(const Utils::Pair val)
	{
		switch (val)
		{
		case Pair::BEGIN_TAB:
			PushTab();
			break;
		case Pair::END_TAB:
			PopTab();
			break;
		case Pair::BEGIN_MAP:
			m_currentType = val;
			m_content << m_tab << MAP_SEPARATOR_BEGIN;
			break;
		case Pair::END_MAP:
			m_currentType = val;
			m_content << m_tab << MAP_SEPARATOR_END;
			break;
		default:
			m_currentType = val;
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
		ParseFile(path);
	}

	Utils::Parser::Parser(const std::string& content)
	{
		ParseContent(content);
	}

	void Utils::Parser::ParseFile(const std::filesystem::path& path)
	{
		auto file = std::fstream(path, std::ios::in);
		if (!file.is_open()) {
			PrintError("File %s could'nt be open", path.string().c_str());
			file.close();
			m_fileOpen = false;
			return;
		}
		// Get file content
		const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		ParseContent(content);

		file.close();
		m_fileOpen = true;
	}

	void Utils::Parser::ParseContent(const std::string& content)
	{
		std::stringstream ss(content);

		std::string line;
		std::unordered_map<std::string, std::string>* currentMap = nullptr;

		while (std::getline(ss, line)) {
			if (line.find(MAP_SEPARATOR_BEGIN) != std::string::npos)
			{
				m_valueMap.emplace_back();
				currentMap = &m_valueMap.back();
			}

			if (const size_t pos = line.find("["); pos != std::string::npos)
			{
				std::string currentKey = line.substr(pos + 1);
				currentKey = currentKey.substr(0, currentKey.find_first_of(']'));
				std::string currentValue = line.substr(line.find_first_of(':') + 2);

				currentMap->insert({ currentKey, currentValue });
			}
		}
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
		ASSERT(m_valueMap[m_currentDepth].contains(key));
		return { m_valueMap[m_currentDepth][key] };
	}

#pragma endregion

	template <typename T>
	T Utils::StringSerializer::As()
	{
		// return static_cast<T>(m_content);
		PrintError("Error with parsing type as");
		return T();
	}

	template <>
	std::string Utils::StringSerializer::As()
	{
		return m_content;
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
	unsigned long long Utils::StringSerializer::As()
	{
		return std::stoull(m_content);
	}

	template <>
	long long Utils::StringSerializer::As()
	{
		return std::stoll(m_content);
	}

	template <>
	unsigned long Utils::StringSerializer::As()
	{
		return std::stoul(m_content);
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
		return { m_content };
	}

	template <>
	Vec3f Utils::StringSerializer::As()
	{
		return { m_content };
	}

	template <>
	Vec4f Utils::StringSerializer::As()
	{
		return { m_content };
	}

	template <>
	Quat Utils::StringSerializer::As()
	{
		return { m_content };
	}

	template <>
	Weak<Core::GameObject> Utils::StringSerializer::As()
	{
		const uint64_t index = As<uint64_t>();
		return Core::SceneHolder::GetCurrentScene()->GetWithUUID(index);
	}

	template <>
	Component::ComponentID Utils::StringSerializer::As()
	{
		uint64_t goIndex;
		uint32_t compIndex;
		std::stringstream os(m_content);
		char skipChar;
		os >> goIndex >> skipChar >> compIndex;

		return { goIndex, compIndex };
	}
}
