#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

namespace GALAXY::Component
{
	class MeshComponent : public IComponent<MeshComponent>
	{
	public:
		MeshComponent() {}
		~MeshComponent() {}

		std::string GetComponentName() const override { return "Mesh Component"; }
		

	private:
		bool testVariable;
		std::string variableString;

		REFLECTION_FRIEND
	};
}
