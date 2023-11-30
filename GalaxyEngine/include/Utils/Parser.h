#pragma once
#include "GalaxyAPI.h"
#include <fstream>
#include <unordered_map>
#include <sstream>
using namespace Utils;
namespace GALAXY
{
	namespace Utils
	{
		enum class Pair
		{
			KEY,
			VALUE,
			BEGIN_TAB,
			END_TAB,
		BEGIN_MAP,
		END_MAP,
		TITLE,
		};

		class Serializer
		{
		public:
			Serializer() {}
			Serializer(const std::filesystem::path& path);
			~Serializer();

			void Create(const std::filesystem::path& path);
			void CloseFile();

			template<typename T>
			Serializer& operator<<(const T& value);

			template<typename T>
			Serializer& operator<<(T* value);

			Serializer& operator<<(const char* value);

			inline void PushTab();
			inline void PopTab();

			inline void AddLine(const std::string& key, const std::string& value);
			inline void AddLine(const std::string& line);

			void SetCurrentType(Utils::Pair val);

			void WriteLine();

			Pair GetCurrentType() const { return m_currentType; }

			void SetCurrentKey(const std::string& key) { m_currentPair.first = key; }
			void SetCurrentValue(const std::string& value) { m_currentPair.second = value; }

			std::string GetContent() const { return m_content.str(); }
		private:
			std::stringstream m_content;
			std::filesystem::path m_filePath = "";

			std::string m_tab = "";

			bool m_first = true;

			std::pair<std::string, std::string> m_currentPair;

			Pair m_currentType = Pair::KEY;
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
			StringSerializer& operator=(StringSerializer&& other) noexcept {
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
			Parser(const std::string& content);
			Parser(const std::filesystem::path& path);

			bool IsFileOpen() const { return m_fileOpen; }

			void ParseFile(const std::filesystem::path& path);
			void ParseContent(const std::string& content);
			void PrintData();

			void NewDepth();

			StringSerializer operator[](const std::string& key);
			const List<UMap<std::string, std::string>>& GetValueMap() const { return m_valueMap; }
			size_t GetCurrentDepth() const { return m_currentDepth; }
		private:
			// Parser
			List<UMap<std::string, std::string>> m_valueMap;

			size_t m_currentDepth = 0;

			bool m_fileOpen = false;
		};

	}
}
#include "Utils/Parser.inl" 
