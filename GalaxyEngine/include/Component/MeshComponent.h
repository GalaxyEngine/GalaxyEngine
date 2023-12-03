#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

namespace GALAXY {
	namespace Resource { class Mesh; class Material; }
	namespace Component
	{
		class GALAXY_API MeshComponent : public IComponent<MeshComponent>
		{
		public:
			MeshComponent() {}
			~MeshComponent() override {}

			inline const char* GetComponentName() const override { return "Mesh Component"; }

			void OnDraw() override;

			inline void SetMesh(const Weak<Resource::Mesh>& mesh) { if (mesh.lock()) { m_mesh = mesh; } }

			void Serialize(Utils::Serializer& serializer) override;
			void Deserialize(Utils::Parser& parser) override;

			void ShowInInspector() override;
		private:
			Weak<Resource::Mesh> m_mesh;

			List<Weak<Resource::Material>> m_materials;

			bool m_drawBoundingBox = false;

			REFLECTION_FRIEND
		};
	}
}
