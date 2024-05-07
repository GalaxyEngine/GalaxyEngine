#pragma once
#include "GalaxyAPI.h"
#include "Debug/Log.h"
#include "Component/Transform.h"
#include "Core/UUID.h"
#include <string>

namespace GALAXY {
	namespace Editor
	{
		class ThumbnailCreator;
		namespace UI {
			class Hierarchy;
			class Inspector;
		}
	}
	namespace Scripting
	{
		class ScriptEngine;
	}
	namespace Resource
	{
		class Prefab;
		class Scene;
	}
	namespace Component
	{
		class Light;
	}
	enum class DrawMode;
	namespace Core {
		class GALAXY_API GameObject : public std::enable_shared_from_this<GameObject>
		{
		public:
			GameObject();
			explicit GameObject(const String& name);
			GameObject& operator=(const GameObject& other) = default;
			GameObject(const GameObject&) = default;
			GameObject(GameObject&&) noexcept = default;
			virtual ~GameObject();


			// === Methods === //
			void StartSelfAndChild() const;
			
			void UpdateSelfAndChild() const;
			void DrawSelfAndChild(DrawMode drawMode) const;

			void RemoveChild(const GameObject* child);
			void RemoveChild(uint32_t index);

			void RemoveFromParent() const;

			Shared<GameObject> Clone() const;

			// This method is to use when you need to destroy over all a gameObject and all its children
			void Destroy();

			void RemoveComponent(Component::BaseComponent* component);

			// === Setters === //
			inline void SetName(String val);

			void AddChild(const Shared<GameObject>& child, uint32_t index = -1);

			// Set the parent to the given GameObject
			void SetParent(const Weak<GameObject>& parent);

			template<typename T>
			inline Weak<T> AddComponent();

			template<typename T>
			inline void AddComponent(Shared<T> component);

			void ChangeComponentIndex(uint32_t prevIndex, uint32_t newIndex);

			// === Getters === //
			inline std::string GetName() const;
			inline UUID GetUUID() const;
			inline uint64_t GetSceneGraphID() const;

			inline Component::Transform* GetTransform() const;
			inline Shared<Core::GameObject> GetParent() const;

			inline uint32_t GetChildrenCount() const;
			inline List<Weak<GameObject>> GetChildren() const;
			List<Weak<GameObject>> GetAllChildren() const;
			inline Weak<GameObject> GetChild(uint32_t index);

			// Return the child index in the list of child
			uint32_t GetChildIndex(const GameObject* child) const;

			template<typename T>
			inline List<Weak<T>> GetComponentsInChildren();
			template<typename T>
			inline List<Weak<T>> GetComponents();
			template<typename T>
			inline Shared<T> GetComponent();
			template<typename T>
			inline Weak<T> GetWeakComponent();

			Weak<Component::BaseComponent> GetComponentWithIndex(uint32_t index);

			Component::BaseComponent* GetComponentWithName(const String& componentName) const;

			// Check if the object given is a parent of the this
			inline bool IsAParent(GameObject* object) const;
			bool IsSibling(const List<Weak<GameObject>>& siblings) const;

			void Serialize(CppSer::Serializer& serializer);
			void Deserialize(CppSer::Parser& parser, bool parseUUID = true);

			inline void SetHierarchyOpen(bool val);

			inline Resource::Scene* GetScene() const;

			// Call after the sceneLoading to synchronize 
			// the components and gameObjects find by the id
			void AfterLoad() const;

			inline bool IsActive() const;
			inline bool IsSelected() const;

			// Prefab methods
			bool IsPrefab() const {return !m_prefab.expired();}
			Weak<Resource::Prefab> GetPrefab() const {return m_prefab;}

		private:
			friend Resource::Scene;
			friend Scripting::ScriptEngine;
			friend Component::Light;
			friend Editor::UI::Inspector;
			friend Editor::ThumbnailCreator;

			UUID m_UUID;
			uint64_t m_sceneGraphID = 0;
			std::string m_name = "GameObject";

			Resource::Scene* m_scene = nullptr;

			Weak<Resource::Prefab> m_prefab = {};
			
			Weak<GameObject> m_parent;
			List<Shared<GameObject>> m_children;
			List<Shared<Component::BaseComponent>> m_components;

			std::unique_ptr<Component::Transform> m_transform;

			// Hierarchy Parameters
			friend Editor::UI::Hierarchy;

			bool m_open = true;
			bool m_selected = false;

			bool m_active = true;

		private:
			template<typename T>
			inline List<Shared<T>> GetComponentsPrivate();
			template<typename T>
			inline List<Shared<T>> GetComponentsInChildrenPrivate();

			template<typename T>
			inline void AddComponent(Shared<T> component, uint32_t index);

			template<typename T>
			inline Weak<T> GetWeakOfComponent(T* component);

			// Set the scene for this object and all its children
			void SetScene(Resource::Scene* scene);
		};

	}
}
#include "Core/GameObject.inl" 
