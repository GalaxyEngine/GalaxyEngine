#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/EditorWindow.h"
#include <deque>

namespace GALAXY 
{
	namespace Debug
	{
		enum class LogType;
	}
	namespace EditorUI
	{
		class Console : public EditorWindow
		{
		public:
			~Console();

			void Draw() override;

			void DisplayText(size_t i);

			void SetResources();

			void AddText(Debug::LogType type, std::string text);

			void Clear();
		private:

		private:
			friend class MainBar;

			const size_t m_maxText = 200;

			std::deque<std::pair<Debug::LogType, std::string>> m_texts;
			size_t m_textSelected = -1;

			std::weak_ptr<Resource::Texture> m_infoTexture;
			std::weak_ptr<Resource::Texture> m_warningTexture;
			std::weak_ptr<Resource::Texture> m_errorTexture;

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
#include "EditorUI/Console.inl" 
