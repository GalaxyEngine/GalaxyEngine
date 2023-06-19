#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

namespace GALAXY {
	namespace Resource { class Mesh; }
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
		private:
			std::weak_ptr<Resource::Mesh> m_mesh;

			REFLECTION_FRIEND
		};
	}
}