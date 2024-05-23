#include "pch.h"
#include "Editor/UI/SceneWindow.h"

#include "Resource/Scene.h"
#include "Core/SceneHolder.h"
#include "Core/Application.h"
#include "Core/Input.h"

#include "Render/Camera.h"
#include "Editor/EditorCamera.h"
#include "Render/Framebuffer.h"

#include "Editor/Gizmo.h"
#include "Editor/UI/EditorUIManager.h"
#include "Resource/Model.h"

#include "Resource/Texture.h"
#include "Resource/ResourceManager.h"
#include "Utils/Time.h"

namespace GALAXY {

	Editor::UI::SceneWindow::~SceneWindow()
	{
	}
	static bool s_modelLoaded = false;
	static bool s_modelFirstLoad = true;

	void Editor::UI::SceneWindow::Draw()
	{
		if (!p_open)
		{
			m_visible = false;
			return;
		}
		if ((m_visible = ImGui::Begin("Scene", &p_open)))
		{
			m_isFocused = ImGui::IsWindowFocused();
			m_isHovered = ImGui::IsWindowHovered();
			SetResources();
			const float windowAvailableWidth = ImGui::GetContentRegionAvail().x;
			if (Wrapper::GUI::TextureButton(m_settingsIcon.lock() ? m_settingsIcon.lock().get() : nullptr, Vec2f(16)))
			{
				ImGui::OpenPopup("Camera settings");
			}
			if (ImGui::BeginPopup("Camera settings", ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
			{
				Core::SceneHolder::GetCurrentScene()->GetEditorCamera()->DisplayCameraSettings();
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			ImGui::Text("FPS %f", 1.f / Utils::Time::DeltaTime());
			ImGui::SameLine();
			ImGui::SetCursorPosX(windowAvailableWidth - 16 * Wrapper::GUI::GetScaleFactor());
			if (Wrapper::GUI::TextureButton(m_menuIcon.lock() ? m_menuIcon.lock().get() : nullptr, Vec2f(16)))
			{
				ImGui::OpenPopup("Menu Icons");
			}
			if (ImGui::BeginPopup("Menu Icons", ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
			{
				ImGui::Checkbox("Draw Grid", Core::Application::GetInstance().GetDrawGridPtr());
				const Shared<Gizmo> gizmo = Core::SceneHolder::GetCurrentScene()->GetGizmo();
				int value = static_cast<int>(gizmo->GetGizmoMode());
				if (ImGui::Combo("Gizmo Mode", &value, Editor::SerializeSpaceEnum()))
				{
					gizmo->SetGizmoMode(static_cast<Space>(value));
				}
				ImGui::EndPopup();
			}
			ImGui::Separator();

			DrawImage();

			UpdateDragModel();
			
			if (ImGui::BeginDragDropTarget()) {
				// When Relased
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {
					FileExplorer* fileExplorer = EditorUIManager::GetInstance()->GetFileExplorer();
					auto draggedFiles = fileExplorer->GetDraggedFiles();
					if (draggedFiles.size() == 1)
					{
						std::shared_ptr<File> file = draggedFiles[0];
						Utils::FileInfo fileInfo = file->GetFileInfo();
						switch (fileInfo.GetResourceType()) {
						case Resource::ResourceType::Scene:
							{
								Core::SceneHolder::OpenScene(fileInfo.GetFullPath());
								break;
							}
						default:
							break;
						}
					}
				}
				// When Hovering
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
					FileExplorer* fileExplorer = EditorUIManager::GetInstance()->GetFileExplorer();
					auto draggedFiles = fileExplorer->GetDraggedFiles();
					if (draggedFiles.size() == 1)
					{
						std::shared_ptr<File> file = draggedFiles[0];
						Utils::FileInfo fileInfo = file->GetFileInfo();
						switch (fileInfo.GetResourceType()) {
						case Resource::ResourceType::Model:
							{
								if (!s_modelFirstLoad) return;
								s_modelFirstLoad = false;
								m_dragModel = Resource::ResourceManager::GetOrLoad<Resource::Model>(fileInfo.GetFullPath());
								m_dragModel.lock()->EOnLoad.Bind([this]() { OnModelLoaded(m_dragModel); });
								if (m_dragModel.lock()->IsLoaded() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
								{
									OnModelLoaded(m_dragModel);
								}
								break;
							}
						default:
							break;
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

		}
		ImGui::End();
	}

	void Editor::UI::SceneWindow::SetResources()
	{
		if (!m_settingsIcon.lock())
		{
			m_settingsIcon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(ENGINE_RESOURCE_FOLDER_NAME"/icons/settings.png");
		}
		if (!m_menuIcon.lock())
		{
			m_menuIcon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(ENGINE_RESOURCE_FOLDER_NAME"/icons/menu.png");
		}
	}

	Math::Vec2f Editor::UI::SceneWindow::GetMousePosition() const
	{
		return (Vec2i)(Input::GetMousePosition() - m_imagePosition);
	}

	void Editor::UI::SceneWindow::UpdateDragModel()
	{
		if (!m_dragModelObject || !s_modelLoaded)
		{
			return;
		}

		auto scene = Core::SceneHolder::GetCurrentScene();
		
		const Physic::Ray ray = scene->GetEditorCamera()->ScreenPointToRay(Vec3f(GetMousePosition(), 10.f));
		auto cameraPosition = ray.origin;
		auto clickPosition = ray.origin + ray.direction * ray.scale;

		m_dragModelObject->GetTransform()->SetWorldPosition(clickPosition);

		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			m_dragModelObject.reset();
			m_dragModel.reset();
			s_modelLoaded = false;
			s_modelFirstLoad = true;
		}
		
	}

	void Editor::UI::SceneWindow::OnModelLoaded(Weak<Resource::Model> model)
	{
		if (s_modelLoaded)
			return;
		s_modelLoaded = true;
		m_dragModelObject = model.lock()->ToGameObject();
		auto scene = Core::SceneHolder::GetCurrentScene();
		scene->GetRootGameObject().lock()->AddChild(m_dragModelObject);
	}

	void Editor::UI::SceneWindow::DrawImage()
	{
		const Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
		if (!currentScene->HasBeenSent())
			return;

		auto editorCamera = currentScene->GetEditorCamera();
		if (!editorCamera)
			return;
		const auto renderTexture = editorCamera->GetRenderTexture().lock().get();
		const auto outlineRenderTexture = editorCamera->GetOutlineFramebuffer()->GetRenderTexture().lock().get();

		if (!renderTexture || !outlineRenderTexture)
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

		m_imagePosition = static_cast<Vec2i>(static_cast<Vec2f>(ImGui::GetWindowPos()) - Core::Application::GetInstance().GetWindow()->GetPosition().ToVec2f()
			+ static_cast<Vec2f>(ImGui::GetCursorPos()));

		Vec2f bottomRight = Vec2f(topLeft.x + width, topLeft.y + height);

		Wrapper::GUI::TextureImage(renderTexture, Vec2f(width, height), { 0, 1 }, { 1, 0 });
		ImGui::SetCursorPos(cursorPos + Vec2f(xPos, yPos));
		Wrapper::GUI::TextureImage(outlineRenderTexture, Vec2f(width, height), { 0, 1 }, { 1, 0 });
		if (drawBorder) {
			auto drawList = ImGui::GetWindowDrawList();
			drawList->AddRect(topLeft, bottomRight, IM_COL32(50, 50, 50, 255), 2.0f, 0, 5.0f); // Gray border
		}
	}

}
