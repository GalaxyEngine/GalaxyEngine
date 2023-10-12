#pragma once
#include "GalaxyAPI.h"
#include <fstream>
#include <unordered_map>

namespace GALAXY
{
	namespace Utils
	{
		enum class PAIR
		{
			KEY,
			VALUE,
			BEGIN_TAB,
			END_TAB,
		};

		class Serializer
		{
		public:
			Serializer(const std::filesystem::path& path);
			~Serializer();

			void Create(const std::filesystem::path& path);
			void CloseFile();

			template<typename T>
			Serializer& operator<<(const T& value);

			Serializer& operator<<(const char* value);

			inline void PushTab();
			inline void PopTab();
			inline void AddLine(const std::string& key, const std::string& value);

			void SetCurrentType(Utils::PAIR val);

			void WriteLine();

			PAIR GetCurrentType() const { return m_currentType; }

			void SetCurrentKey(const std::string& key) { m_currentPair.first = key; }
			void SetCurrentValue(const std::string& value) { m_currentPair.second = value; }
		private:
			// Serializer
			std::ofstream m_file;

			std::string m_tab = "";

			bool m_first = true;

			std::pair<std::string, std::string> m_currentPair;

			PAIR m_currentType = PAIR::KEY;
		};

		class StringSerializer
		{
		private:
			std::string m_content;
		public:
			StringSerializer(std::string value) : m_content(value) {}

			StringSerializer& operator=(const std::string& str) {
				m_content = str;
				return *this;
			}

			StringSerializer& operator=(const char* cstr) {
				m_content = cstr;
				return *this;
			}

			StringSerializer& operator=(char c) {
				m_content = c;
				return *this;
			}

			StringSerializer& operator=(const StringSerializer& other) {
				if (this != &other) {
					m_content = other.m_content;
				}
				return *this;
			}

			// Custom operator= for move assignment
			StringSerializer& operator=(StringSerializer&& other) {
				if (this != &other) {
					m_content = std::move(other.m_content);
				}
				return *this;
			}

			operator std::string() const {
				return m_content;
			}

			template <typename T>
			T As();
		};

		class Parser
		{
		public:
			Parser(const std::filesystem::path& path);

			void Parse(const std::filesystem::path& path);
			void PrintData();

			void NewDepth();

			StringSerializer operator[](const std::string& key);
		private:
			// Parser
			std::fstream m_file;
			std::vector<std::unordered_map<std::string, std::string>> m_valueMap;

			size_t m_currentDepth = 0;
		};

	}
}
#include "Utils/Parser.inl" 
