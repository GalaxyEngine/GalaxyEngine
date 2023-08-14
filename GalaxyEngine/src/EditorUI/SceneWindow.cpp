#include "pch.h"
#include "EditorUI/SceneWindow.h"

#include "Core/Scene.h"
#include "Core/SceneHolder.h"
#include "Render/Camera.h"

#include "Resource/Texture.h"
#include "Resource/ResourceManager.h"

namespace GALAXY {

	EditorUI::SceneWindow::~SceneWindow()
	{
	}

	void EditorUI::SceneWindow::Draw()
	{
		if (!p_open)
			return;
		if (m_visible = ImGui::Begin("Scene", &p_open))
		{
			m_isHovered = ImGui::IsWindowHovered();
			if (!m_settingsIcon.lock())
			{
				m_settingsIcon = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(ENGINE_RESOURCE_FOLDER_NAME"/icons/settings.png");
			}
			if (Wrapper::GUI::TextureButton(m_settingsIcon.lock() ? m_settingsIcon.lock().get() : nullptr, Vec2f(16)))
			{
				ImGui::OpenPopup("Camera settings");
			}
			if (ImGui::BeginPopup("Camera settings", ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
			{
				Core::SceneHolder::GetCurrentScene()->GetEditorCamera()->DisplayCameraSettings();
				ImGui::EndPopup();
			}
			ImGui::Separator();
			

			DrawImage();

		}
		ImGui::End();
	}

	void EditorUI::SceneWindow::DrawImage()
	{
		auto renderTexture = Core::SceneHolder::GetCurrentScene()->GetEditorCamera()->GetRenderTexture().lock().get();

		float contentWidth = ImGui::GetContentRegionAvail().x;
		float contentHeight = ImGui::GetContentRegionAvail().y;

		const bool keepAspectRatio = true;
		const bool drawBorder = false;

		float width = contentWidth, height = contentHeight;

		if (keepAspectRatio) {
			float aspectRatio = 16.f / 9.f;

			// Calculate dimensions while maintaining the aspect ratio
			if (contentWidth / aspectRatio <= contentHeight)
			{
				width = contentWidth;
				height = width / aspectRatio;
			}
			else
			{
				height = contentHeight;
				width = height * aspectRatio;
			}
		}
		// Calculate the x position for centering horizontally
		float xPos = (ImGui::GetContentRegionAvail().x - width) * 0.5f;
		float yPos = (ImGui::GetContentRegionAvail().y - height) * 0.5f;

		// Set the cursor position to center the content horizontally
		ImGui::SetCursorPos(ImGui::GetCursorPos() + Vec2f(xPos, yPos));

		// Get Position to draw a border
		Vec2f topLeft = ImGui::GetCursorScreenPos();
		Vec2f bottomRight = Vec2f(topLeft.x + width, topLeft.y + height);

		Wrapper::GUI::TextureImage(renderTexture, Vec2f(width, height), { 0, 1 }, { 1, 0 });
		if (drawBorder) {
			auto drawList = ImGui::GetWindowDrawList();
			drawList->AddRect(topLeft, bottomRight, IM_COL32(50, 50, 50, 255), 2.0f, 0, 5.0f); // Gray border
		}
	}

}