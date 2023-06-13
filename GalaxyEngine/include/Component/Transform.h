#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"
namespace GALAXY::Component {
	class Transform : public IComponent<Transform>
	{
	public:
		Transform() {}
		~Transform() {}

		std::string GetComponentName() override { return "Transform"; }

	private:

	};
}
