#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

namespace GALAXY {
	namespace Resource { class Mesh; class Material; }
	namespace Component
	{
		class MeshComponent : public IComponent<MeshComponent>
		{
		public:
			MeshComponent() {}
			~MeshComponent() {}

			std::string GetComponentName() const override { return "Mesh Component"; }

			void OnDraw() override;

			void SetMesh(const std::weak_ptr<Resource::Mesh>& mesh);

			void ShowInInspector() override;
		private:
			std::weak_ptr<Resource::Mesh> m_mesh;
			std::vector<std::weak_ptr<Resource::Material>> m_materials;

			REFLECTION_FRIEND
		};
	}
}