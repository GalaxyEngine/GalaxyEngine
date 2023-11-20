#include "pch.h"
#include "Editor/EditorIcon.h"

#include "Resource/ResourceManager.h"

#include "Utils/Define.h"

#include "Core/SceneHolder.h"

namespace GALAXY 
{

	Editor::EditorIcon::EditorIcon()
	{
		m_material = std::make_shared<Resource::Material>("Icon Material");
		auto billboardShader = Resource::ResourceManager::GetOrLoad<Resource::Shader>(BILLBOARD_PATH);
		m_material->SetShader(billboardShader);
		m_material->SetAmbient(Vec4f(1));
		m_material->SetDiffuse(Vec4f(1));
		m_material->SetSpecular(Vec4f(1));
		m_plane = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(PLANE_PATH);
	}

	void Editor::EditorIcon::SetIconTexture(Weak<Resource::Texture> iconTexture)
	{
		m_material->SetAlbedo(iconTexture);
	}

	void Editor::EditorIcon::Render(const Mat4& model, uint64_t id /*= -1*/)
{
		static Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		// Don't render outline of the plane because it's ugly
		if (renderer->GetRenderType() == Render::RenderType::Outline)
			return;

		ASSERT(m_plane.lock());
		ASSERT(m_material);
		ASSERT(m_material->GetShader());

		m_plane.lock()->Render(model, { m_material }, id);
	}

}
