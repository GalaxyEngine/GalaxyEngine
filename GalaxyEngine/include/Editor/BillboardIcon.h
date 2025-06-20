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
		class BillboardIcon
		{
		public:
			BillboardIcon();
			BillboardIcon& operator=(const BillboardIcon& other) = default;
			BillboardIcon(const BillboardIcon&) = default;
			BillboardIcon(BillboardIcon&&) noexcept = default;
			virtual ~BillboardIcon() = default;

			void SetIconTexture(const Weak<Resource::Texture>& iconTexture) const;

			void Render(uint64_t id = -1);

			void SetPosition(const Vec3f& position);
		private:
			Weak<Resource::Mesh> m_plane;
			Shared<Resource::Material> m_material;

			Vec3f m_currentPosition = Vec3f{ -1 };
			Mat4 m_translationMatrix = Mat4::Identity();
		};
	}
}
