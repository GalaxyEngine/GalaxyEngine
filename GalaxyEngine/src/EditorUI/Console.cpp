#include "pch.h"
#include "EditorUI/Console.h"

#include "Resource/ResourceManager.h"

namespace GALAXY {
	EditorUI::Console::Console()
	{
	}

	void EditorUI::Console::Draw()
	{
		if (!p_open)
			return;
		if (ImGui::Begin("Console", &p_open, ImGuiWindowFlags_NoTitleBar))
		{
			SetResources();

			const Vec2f size(24, 24);
			if (ImGui::Button("Clear", {0, size.y}))
			{
				Clear();
			}
			ImGui::SameLine();
			ImGui::PushID(0);
			Wrapper::GUI::TextureToggleButtonWithText(m_infoTexture.lock().get(), std::to_string(m_infoNumber).c_str(), &m_infoCheckbox, size);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID(1);
			Wrapper::GUI::TextureToggleButtonWithText(m_warningTexture.lock().get(), std::to_string(m_warningNumber).c_str(), &m_warningCheckbox, size);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID(2);
			Wrapper::GUI::TextureToggleButtonWithText(m_errorTexture.lock().get(), std::to_string(m_errorNumber).c_str(), &m_errorCheckbox, size);
			ImGui::PopID();

			static ImGuiTextFilter filter;
			ImGui::SameLine();
			filter.Draw("Search");

			// Setting Splitter for the log and the text selected
			static float size1 = ImGui::GetContentRegionAvail().y - (25 * ImGui::GetWindowHeight() / 100), size2 = 25 * ImGui::GetWindowHeight() / 100;
			Wrapper::GUI::Splitter(false, 2, &size1, &size2, 10, 10);
			ImGui::BeginChild("Content", Vec2f(0, size1), true);
			for (size_t i = 0; i < m_texts.size(); i++)
			{
				if (filter.PassFilter(m_texts[i].second.c_str()))
					DisplayText(i);
			}
			ImGui::EndChild();

			// Display Text Selected
			ImGui::BeginChild("Message", {0, size2 });
			if (m_textSelected != -1 && m_textSelected < m_texts.size())
			{
				std::string prefix = "";
				Vec4f color = Vec4f(1);
				switch (m_texts[m_textSelected].first)
				{
				case Debug::LogType::L_INFO:
					prefix = "(Info)";
					break;
				case Debug::LogType::L_WARNING:
					color = Vec4f(1, 1, 0, 1);
					prefix = "(Warning)";
					break;
				case Debug::LogType::L_ERROR:
					color = Vec4f(1, 0, 0, 1);
					prefix = "(Error)";
					break;
				}
				auto currentText = m_texts[m_textSelected].second;
				auto pos = currentText.find("):") + 1;
				std::string message = currentText.substr(pos + 1);
				std::string fileLine = currentText.substr(currentText.find_first_of(']') + 2, pos - currentText.find_first_of(']') - 2);
				Wrapper::GUI::TextSelectable(prefix + message, color);
				Wrapper::GUI::TextSelectable(fileLine);
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}

	void EditorUI::Console::DisplayText(size_t i)
	{
		ImGui::PushID(i);
		Resource::Texture* tex = nullptr;
		switch (m_texts[i].first)
		{
		case Debug::LogType::L_INFO:
			if (!m_infoCheckbox)
				return;
			tex = m_infoTexture.lock().get();
			break;
		case Debug::LogType::L_WARNING:
			if (!m_warningCheckbox)
				return;
			tex = m_warningTexture.lock().get();
			break;
		case Debug::LogType::L_ERROR:
			if (!m_errorCheckbox)
				return;
			tex = m_errorTexture.lock().get();
			break;
		default:
			break;
		}
		if (tex) {
			Wrapper::GUI::TextureImage(tex, Vec2f(32));
			ImGui::SameLine();
		}
		if (ImGui::Selectable(m_texts[i].second.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, Vec2f(0, 32)))
			m_textSelected = i;
		ImGui::PopID();
	}

	void EditorUI::Console::SetResources()
	{
		if (m_resourcesLoaded)
			return;
		if (!m_infoTexture.lock())
		{
			m_infoTexture = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(ENGINE_RESOURCE_FOLDER_NAME"/icons/info.png");
		}
		if (!m_warningTexture.lock())
		{
			m_warningTexture = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(ENGINE_RESOURCE_FOLDER_NAME"/icons/warning.png");
		}
		if (!m_errorTexture.lock())
		{
			m_errorTexture = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(ENGINE_RESOURCE_FOLDER_NAME"/icons/error.png");
		}

		m_resourcesLoaded = m_infoTexture.lock() && m_warningTexture.lock() && m_errorTexture.lock();
		PrintLog("Aleddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\naaa\naaa\naaa\naaa\naaa\naaa");

	}

	void EditorUI::Console::AddText(Debug::LogType type, std::string text)
	{
		if (m_texts.size() > m_maxText)
		{
			for (size_t i = 0; i < m_texts.size() - m_maxText + 1; i++)
			{
				switch (m_texts[i].first)
				{
				case Debug::LogType::L_INFO:
					m_infoNumber--;
					break;
				case Debug::LogType::L_WARNING:
					m_warningNumber--;
					break;
				case Debug::LogType::L_ERROR:
					m_errorNumber--;
					break;
				}
			}
			m_texts.erase(m_texts.begin(), m_texts.begin() + m_texts.size() - m_maxText + 1);
		}

		switch (type)
		{
		case Debug::LogType::L_INFO:
			m_infoNumber++;
			break;
		case Debug::LogType::L_WARNING:
			m_warningNumber++;
			break;
		case Debug::LogType::L_ERROR:
			m_errorNumber++;
			break;
		}

		m_texts.push_back(std::make_pair(type, text));
	}

	void EditorUI::Console::Clear()
	{
		m_texts.clear();
		m_textSelected = -1;
		m_infoNumber = 0;
		m_warningNumber = 0;
		m_errorNumber = 0;
	}
}