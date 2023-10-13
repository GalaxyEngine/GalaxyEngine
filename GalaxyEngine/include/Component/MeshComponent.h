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
			~MeshComponent() {}

			const char* GetComponentName() const override { return "Mesh Component"; }

			virtual std::vector<const char*> GetComponentNames() const override\
			{
				auto vector = BaseComponent::GetComponentNames();
				vector.insert(vector.end(), MeshComponent::GetComponentName());
				return vector;
			}

			void OnDraw() override;

			void SetMesh(const std::weak_ptr<Resource::Mesh>& mesh);

			void Serialize(Utils::Serializer& serializer) override;
			void Deserialize(Utils::Parser& parser) override;

			void ShowInInspector() override;
		private:
			std::weak_ptr<Resource::Mesh> m_mesh;
			std::vector<std::weak_ptr<Resource::Material>> m_materials;

			REFLECTION_FRIEND
		};
	}
}