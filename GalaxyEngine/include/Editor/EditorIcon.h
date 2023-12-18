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
			virtual ~EditorIcon() = default;

			void SetIconTexture(const Weak<Resource::Texture>& iconTexture) const;

			void Render(uint64_t id = -1);

			void SetPosition(const Vec3f& position);
		private:
			Weak<Resource::Mesh> m_plane;
			Shared<Resource::Material> m_material;

			Vec3f m_currentPosition;
			Mat4 m_translationMatrix;
		};
	}
}
