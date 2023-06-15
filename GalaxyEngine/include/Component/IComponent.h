#pragma once
#include "GalaxyAPI.h"
namespace GALAXY {
	namespace Core { class GameObject; }
	namespace Component {

		class BaseComponent
		{
		public:
			std::weak_ptr<Core::GameObject> gameObject;

		public:
			BaseComponent() {}
			BaseComponent& operator=(const BaseComponent& other) = default;
			BaseComponent(const BaseComponent&) = default;
			BaseComponent(BaseComponent&&) noexcept = default;
			virtual ~BaseComponent() {}

			virtual std::string GetComponentName() { return "Empty"; };

			virtual void ShowInInspector();

			virtual void OnCreate() {}

			virtual void OnStart() {}

			virtual void OnUpdate() {}

			virtual void OnDraw() {}

			virtual void OnDestroy() {}

			bool IsEnable() { return p_enable; }

		protected:
			bool p_enable = true;

		};

		template <typename Derived>
		class IComponent : public BaseComponent
		{
		public:
			IComponent() {}
			IComponent& operator=(const IComponent& other) = default;
			IComponent(const IComponent&) = default;
			IComponent(IComponent&&) noexcept = default;
			virtual ~IComponent() {}

			virtual std::string GetComponentName() override = 0;

			virtual Derived* Clone() const {
				return new Derived(static_cast<Derived const&>(*this));
			}
		private:

		};

	}
}