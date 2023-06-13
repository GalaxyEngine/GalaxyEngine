#pragma once
#include "GalaxyAPI.h"
namespace GALAXY {
	namespace Core { class GameObject; }
	namespace Component {

		template <typename Derived>
		class IComponent
		{
		public:
			IComponent() {}
			IComponent& operator=(const IComponent& other) = default;
			IComponent(const IComponent&) = default;
			IComponent(IComponent&&) noexcept = default;
			virtual ~IComponent() {}

			virtual std::string GetComponentName() = 0;

			virtual void OnCreate() {}

			virtual void OnStart() {}

			virtual void OnUpdate() {}

			virtual void OnDraw() {}

			virtual void OnDestroy() {}

			virtual Derived* Clone() const {
				return new Derived(static_cast<Derived const&>(*this));
			}

			std::weak_ptr<Core::GameObject> gameObject;

		private:

		};
	}
}