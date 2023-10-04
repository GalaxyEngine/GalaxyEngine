#pragma once
#include "GalaxyAPI.h"
#include <Wrapper/Reflection.h>
#include <vector>

namespace GALAXY {
	namespace Core { class GameObject; }
	namespace Component {

		class GALAXY_API BaseComponent
		{
		public:
			Weak<Core::GameObject> gameObject = Weak<Core::GameObject>();

		public:
			BaseComponent();
			BaseComponent& operator=(const BaseComponent& other) = default;
			BaseComponent(const BaseComponent&) = default;
			BaseComponent(BaseComponent&&) noexcept = default;
			virtual ~BaseComponent() {}

			virtual const char* GetComponentName() const { return "BaseComponent"; }

			virtual std::vector<const char*> GetComponentNames() const;

			virtual void ShowInInspector() {}

			virtual void OnCreate() {}

			virtual void OnStart() {}

			virtual void OnUpdate() {}

			virtual void OnDraw() {}

			virtual void OnDestroy() {}

			void RemoveFromGameObject();

			virtual void Reset() {}

			// === Getters === //

			bool IsEnable() const { return p_enable; }

			virtual void SetGameObject(Weak<Core::GameObject> object) { gameObject = object; }

			std::shared_ptr<Core::GameObject> GameObject() { return gameObject.lock(); }

			// === Setters === //

			void SetEnable(bool enable) { p_enable = enable; }


			virtual std::shared_ptr<BaseComponent> Clone() = 0;

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

			virtual std::shared_ptr<BaseComponent> Clone() override {
				return std::make_shared<Derived>(*dynamic_cast<Derived*>(this));
			}

			// Reset All the value of the component.
			void Reset() override
			{
				*static_cast<Derived*>(this) = Derived();
			}

			virtual void ShowInInspector() override
			{
#ifdef ENABLE_REFLECTION
				Wrapper::Reflection::ShowInspectorClass(dynamic_cast<Derived*>(this));
#endif
			}
		private:

		};

	}
}