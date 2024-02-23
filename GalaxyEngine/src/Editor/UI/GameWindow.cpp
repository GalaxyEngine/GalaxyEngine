#include "pch.h"
#include "Editor/UI/GameWindow.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"

#include "Resource/Scene.h"

#include "Render/Camera.h"

#include "Component/CameraComponent.h"

void Editor::UI::GameWindow::Draw()
{
	if (!p_open)
	{
		m_visible = false;
		return;
	}
	if ((m_visible = ImGui::Begin("Game", &p_open)))
	{
		DrawImage();
	}
	ImGui::End();
}

void Editor::UI::GameWindow::DrawImage()
{
	const Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
	if (!currentScene->HasBeenSent())
		return;
	Shared<Component::CameraComponent> mainCamera = currentScene->GetMainCamera();
	if (!mainCamera)
		return;
	const auto renderTexture = mainCamera->GetRenderTexture().lock().get();

	if (!renderTexture)
		return;

	const float contentWidth = ImGui::GetContentRegionAvail().x;
	const float contentHeight = ImGui::GetContentRegionAvail().y;

	constexpr bool keepAspectRatio = true;
	constexpr bool drawBorder = false;

	float width = contentWidth, height = contentHeight;

	if (keepAspectRatio) {
		const float aspectRatio = 16.f / 9.f;

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
	m_imageSize = Vec2f(width, height);
	// Calculate the x position for centering horizontally
	const float xPos = (ImGui::GetContentRegionAvail().x - width) * 0.5f;
	const float yPos = (ImGui::GetContentRegionAvail().y - height) * 0.5f;

	const auto cursorPos = ImGui::GetCursorPos();
	// Set the cursor position to center the content horizontally
	ImGui::SetCursorPos(cursorPos + Vec2f(xPos, yPos));

	// Get Position to draw a border
	const Vec2f topLeft = ImGui::GetCursorScreenPos();

	Vec2f m_imagePosition = static_cast<Vec2i>(static_cast<Vec2f>(ImGui::GetWindowPos()) - Core::Application::GetInstance().GetWindow()->GetPosition().ToVec2f()
		+ static_cast<Vec2f>(ImGui::GetCursorPos()));

	Vec2f bottomRight = Vec2f(topLeft.x + width, topLeft.y + height);

	Wrapper::GUI::TextureImage(renderTexture, Vec2f(width, height), { 0, 1 }, { 1, 0 });
	if (drawBorder) {
		auto drawList = ImGui::GetWindowDrawList();
		drawList->AddRect(topLeft, bottomRight, IM_COL32(50, 50, 50, 255), 2.0f, 0, 5.0f); // Gray border
	}
}