#pragma once
#include "GalaxyAPI.h"
#include "Utils/Type.h"
namespace GALAXY 
{
	namespace Resource
	{
		class Mesh;
		class Material;
		class Texture;
	}
	namespace Editor
	{
		class EditorIcon
		{
		public:
			EditorIcon();
			EditorIcon& operator=(const EditorIcon& other) = default;
			EditorIcon(const EditorIcon&) = default;
			EditorIcon(EditorIcon&&) noexcept = default;
			virtual ~EditorIcon() {}

			void SetIconTexture(Weak<Resource::Texture> iconTexture);

			void Render(const Mat4& model, uint64_t id = -1);
		private:
			Weak<Resource::Mesh> m_plane;
			Shared<Resource::Material> m_material;
		};
	}
}
