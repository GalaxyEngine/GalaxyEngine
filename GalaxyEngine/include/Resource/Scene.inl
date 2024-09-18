#pragma once
#include <ranges>

#include "Resource/Scene.h"
namespace GALAXY
{
	template<typename... Args> inline Weak<Core::GameObject> Resource::Scene::CreateObject(Args&&... args)
	{
		std::shared_ptr<Core::GameObject> shared = std::make_shared<Core::GameObject>(std::forward<Args>(args)...);
		shared->m_scene = this;

		AddObject(shared);

		m_lastAdded.push_back(shared);

		return shared;
	}

	inline void Resource::Scene::AddObject(const std::shared_ptr<Core::GameObject>& gameObject)
	{
		if (!m_objectList.contains(gameObject->m_UUID))
		{
			m_objectList[gameObject->m_UUID] = gameObject;
			gameObject->SetScene(this);
		}
		else
		{
			ASSERT(false);
		}

		for (auto& child : gameObject->GetChildren())
		{
			AddObject(child.lock());
		}
	}

	inline void Resource::Scene::RemoveObject(Core::GameObject* object)
	{
		if (m_objectList.empty() || !object)
			return;
		const auto shared = m_objectList.find(object->m_UUID);
		if (shared != m_objectList.end()) {
			if (shared != m_objectList.end())
			{
				m_objectList.erase(shared);
			}
		}
	}

	inline Weak<GALAXY::Core::GameObject> Resource::Scene::GetWithSceneGraphID(const uint64_t index)
	{
		for (auto& object : m_objectList | std::views::values)
		{
			if (object->GetSceneGraphID() == index)
			{
				return object;
			}
		}
		return {};
	}

	inline Weak<GALAXY::Core::GameObject> Resource::Scene::GetWithUUID(const Core::UUID& uuid)
	{
		if (m_objectList.contains(uuid))
		{
			return m_objectList.at(uuid);
		}
		return {};
	}

	inline Weak<Core::GameObject> Resource::Scene::GetRootGameObject() const
	{
		return m_root;
	}

	inline const Mat4& Resource::Scene::GetView() const
	{
		return m_view;
	}

	inline const Mat4& Resource::Scene::GetProjection() const
	{
		return m_projection;
	}

	inline const Mat4& Resource::Scene::GetVP() const
	{
		return m_VP;
	}

	inline const Vec3f& Resource::Scene::GetCameraUp() const
	{
		return m_cameraUp;
	}

	inline const Vec3f& Resource::Scene::GetCameraRight() const
	{
		return m_cameraRight;
	}

	inline Shared<Render::Camera> Resource::Scene::GetCurrentCamera() const
	{
		return m_currentCamera.lock();
	}

	inline Shared<Component::CameraComponent> Resource::Scene::GetMainCamera() const
	{
		return m_mainCamera.lock();
	}

	inline const UMap<Core::UUID, Shared<Core::GameObject>>& Resource::Scene::GetObjectList() const
	{
		return m_objectList;
	}

	inline void Resource::Scene::SetData(const Resource::Scene* other)
	{
		m_root = other->m_root;
		m_lightManager = other->m_lightManager;
		m_cameras = other->m_cameras;
		m_mainCamera = other->m_mainCamera;
		m_objectList = other->m_objectList;
		
		// This will set the scene for all children as well
		m_root->SetScene(this);
	}

#ifdef WITH_EDITOR
	inline void Resource::Scene::RevertObject(size_t number)
	{
		for (size_t i = 0; i < number; ++i)
		{
			auto last = m_lastAdded.back();
			last.lock()->Destroy();
			m_lastAdded.pop_back();
		}
	}

	inline Shared<Render::EditorCamera> Resource::Scene::GetEditorCamera() const
	{
		return m_editorCamera;
	}

	inline Shared<Editor::Gizmo> Resource::Scene::GetGizmo() const
	{
		return m_gizmo;
	}

	inline Shared<Editor::ActionManager> Resource::Scene::GetActionManager() const
	{
		return m_actionManager;
	}
#endif
}
