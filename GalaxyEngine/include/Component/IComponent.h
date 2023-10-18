#pragma once
#include "GalaxyAPI.h"
#include <Wrapper/Reflection.h>

#include <vector>
#include <unordered_map>

namespace GALAXY {
	namespace Utils { class Serializer; class Parser; }
	namespace Core { class GameObject; }
	namespace Component {

		struct ComponentID
		{
			uint64_t gameObjectID;
			uint32_t componentID;
		};

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

			virtual void AfterLoad() {}

			void RemoveFromGameObject();

			virtual void Reset() {}

			virtual void Serialize(Utils::Serializer& serializer) {}
			virtual void Deserialize(Utils::Parser& parser) {}

			// === Getters === //

			bool IsEnable() const { return p_enable; }

			virtual void SetGameObject(Weak<Core::GameObject> object) { gameObject = object; }

			std::shared_ptr<Core::GameObject> GameObject() { return gameObject.lock(); }

			uint32_t GetIndex() const { return p_id; }

			// === Setters === //

			void SetEnable(bool enable) { p_enable = enable; }


			virtual std::shared_ptr<BaseComponent> Clone() = 0;

		protected:
			friend Core::GameObject;

			bool p_enable = true;

			uint32_t p_id = -1;

			// De-serialization variables
			std::unordered_map<std::string, uint64_t> p_tempGameObjectIDs;
			std::unordered_map<std::string, ComponentID> p_tempComponentIDs;
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