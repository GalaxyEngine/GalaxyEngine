#include "pch.h"
#include "EditorUI/SceneWindow.h"

#include "Core/Scene.h"
#include "Core/SceneHolder.h"
#include "Render/Camera.h"
#include "Resource/Texture.h"

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
			auto renderTexture = Core::SceneHolder::GetCurrentScene()->GetEditorCamera()->GetRenderTexture().lock().get();

			Wrapper::GUI::TextureImage(renderTexture, (Vec2f)ImGui::GetContentRegionAvail(), {0, 1}, {1, 0});
		}
		ImGui::End();
	}

}