#pragma once
#include "GalaxyAPI.h"
#include "Utils/Type.h"
#include "Core/UUID.h"
#include "Utils/Define.h"

#include "Wrapper/Reflection.h"

namespace GALAXY {
	namespace Utils { class Serializer; class Parser; }
	namespace Core { class GameObject; }
	namespace Component {

		struct GALAXY_API ComponentID
		{
			Core::UUID gameObjectID;
			uint32_t componentID = INDEX_NONE;
		};

		class GALAXY_API BaseComponent
		{
		public:
			BaseComponent();
			BaseComponent& operator=(const BaseComponent& other) = default;
			BaseComponent(const BaseComponent&) = default;
			BaseComponent(BaseComponent&&) noexcept = default;
			virtual ~BaseComponent() {}

			// Return the component name
			inline virtual const char* GetComponentName() const { return "BaseComponent"; }

			// Return the list of component names
			inline virtual List<const char*> GetComponentNames() const {
				List<const char*> names;
				names.push_back(BaseComponent::GetComponentName());
				names.push_back(GetComponentName());
				return names;
			}

			virtual void ShowInInspector() {}

			// Called on Creation
			virtual void OnCreate() {}

			// Called on Start Game
			virtual void OnStart() {}

			// Called on Game Update
			virtual void OnUpdate() {}

			// Called on Editor Update
			virtual void OnEditorUpdate() {}

			// Called on Game Draw
			virtual void OnDraw() {}

			// Called on Editor Draw
			virtual void OnEditorDraw() {}

			// Called before being Destroyed
			virtual void OnDestroy() {}

			// Called just after Loading the scene
			virtual void AfterLoad() {}

			void RemoveFromGameObject();

			virtual void Reset() {}

			virtual void Serialize(Utils::Serializer& serializer) {}
			virtual void Deserialize(Utils::Parser& parser) {}

			// ========= Getters ========= //

			// return if the component is enable and his gameobject
			[[nodiscard]] bool IsEnable() const;

			// return if the component is enable and only him
			inline bool IsSelfEnable() const { return p_enable; }

			inline virtual void SetGameObject(const Weak<Core::GameObject>& object) { p_gameObject = object; }

			inline Shared<Core::GameObject> GetGameObject() const { return p_gameObject.lock(); }

			class Transform* GetTransform() const;

			inline uint32_t GetIndex() const { return p_id; }

			// ========= Setters ========= //

			inline void SetSelfEnable(const bool enable) { p_enable = enable; }

			virtual Shared<BaseComponent> Clone() = 0;

		protected:
			friend Core::GameObject;

			Weak<Core::GameObject> p_gameObject = Weak<Core::GameObject>();

			bool p_enable = true;

			uint32_t p_id = -1;

			// De-serialization variables
			std::unordered_map<std::string, uint64_t> p_tempGameObjectIDs;
			std::unordered_map<std::string, ComponentID> p_tempComponentIDs;
		};

		template <typename Derived>
		class GALAXY_API IComponent : public BaseComponent
		{
		public:
			IComponent() = default;
			IComponent& operator=(const IComponent& other) = default;
			IComponent(const IComponent&) = default;
			IComponent(IComponent&&) noexcept = default;
			~IComponent() override = default;

			// Clone the component
			inline virtual Shared<BaseComponent> Clone() override {
				return std::make_shared<Derived>(*dynamic_cast<Derived*>(this));
			}

			// Reset All the value of the component.
			inline void Reset() override
			{
				*static_cast<Derived*>(this) = Derived();
			}

			inline virtual void ShowInInspector() override
			{
#ifdef ENABLE_REFLECTION
				Wrapper::Reflection::ShowInspectorClass(dynamic_cast<Derived*>(this));
#endif
			}
		private:

			};

		}
	}