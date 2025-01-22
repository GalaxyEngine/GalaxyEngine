#pragma once
#include "GalaxyAPI.h"
#include "Editor/UI/EditorWindow.h"
#include <deque>

namespace GALAXY
{
	namespace Debug
	{
		enum class LogType;
		struct LogText
		{
			LogType type;
			std::string text;

			LogText(const LogType type, std::string text) : type(type), text(std::move(text)) {}
		};
	}
	namespace Editor::UI
	{
		class Console : public EditorWindow
		{
		public:
			~Console() override = default;

			void Draw() override;

			void DisplayText(size_t i);

			void SetResources();

			void AddText(Debug::LogType type, const std::string& text);

			void Clear();
		private:

		private:
			friend class MainBar;

			const size_t m_maxText = 200;

			std::deque<Debug::LogText> m_texts;
			std::deque<Debug::LogText*> m_displayText;
			size_t m_textSelected = -1;

			Weak<Resource::Texture> m_infoTexture;
			Weak<Resource::Texture> m_warningTexture;
			Weak<Resource::Texture> m_errorTexture;

			size_t m_infoNumber = 0;
			size_t m_warningNumber = 0;
			size_t m_errorNumber = 0;

			bool m_infoCheckbox = true;
			bool m_warningCheckbox = true;
			bool m_errorCheckbox = true;

			bool m_resourcesLoaded = false;

			bool m_scrollToBottom = false;
		};
	}
}
#include "Editor/UI/Console.inl" 
