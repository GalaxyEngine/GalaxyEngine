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

			inline const char* GetComponentName() const override { return "MeshComponent"; }

			void OnEditorDraw() override;
			void OnDraw() override;

			inline void SetMesh(const Weak<Resource::Mesh>& mesh) { if (mesh.lock()) { m_mesh = mesh; } }
			inline Weak<Resource::Mesh> GetMesh() const { return m_mesh; }

			void Serialize(CppSer::Serializer& serializer) override;
			void Deserialize(CppSer::Parser& parser) override;

			void AddMaterial(const Weak<Resource::Material>& material);
			void RemoveMaterial(size_t index);
			List<Weak<Resource::Material>> GetMaterials() const { return m_materials; }
			void ClearMaterials();

			void ShowInInspector() override;
		private:
			Weak<Resource::Mesh> m_mesh;
			List<Weak<Resource::Material>> m_materials;

			bool m_drawBoundingBox = false;
			bool m_drawModelBoundingBox = false;
		};
	}
}
