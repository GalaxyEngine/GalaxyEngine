#include "pch.h"
#include "Editor/EditorIcon.h"

#include "Resource/ResourceManager.h"

#include "Utils/Define.h"

#include "Core/SceneHolder.h"
#include "Resource/Mesh.h"

namespace GALAXY
{

	Editor::EditorIcon::EditorIcon()
	{
		// TODO : Only one editor icon per icon
		m_material = std::make_shared<Resource::Material>("Icon Material");
		const Weak<Resource::Shader> billboardShader = Resource::ResourceManager::GetOrLoad<Resource::Shader>(BILLBOARD_PATH);
		m_material->SetShader(billboardShader);
		m_material->SetAmbient(Vec4f(1));
		m_material->SetDiffuse(Vec4f(1));
		m_material->SetSpecular(Vec4f(1));
		m_plane = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(PLANE_PATH);
	}

	void Editor::EditorIcon::SetIconTexture(const Weak<Resource::Texture>& iconTexture) const
	{
		m_material->SetAlbedo(iconTexture);
	}

	void Editor::EditorIcon::SetPosition(const Vec3f& position)
	{
		if (m_currentPosition == position)
			return;
		m_currentPosition = position;
		m_translationMatrix = Mat4::CreateTranslationMatrix(m_currentPosition);
	}

	void Editor::EditorIcon::Render(uint64_t id/*= -1*/)
	{
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		// Don't render outline of the plane because it's ugly
		if (renderer->GetRenderType() == Render::RenderType::Outline)
			return;

		ASSERT(m_plane.lock());
		ASSERT(m_material);
		ASSERT(m_material->GetShader());

		m_plane.lock()->Render(m_translationMatrix, { m_material }, id);
	}

}
