#pragma once
#include <ranges>

#include "Resource/Scene.h"
namespace GALAXY
{
	template<typename... Args> inline Weak<Core::GameObject> Resource::Scene::CreateObject(Args&&... args)
	{
		std::shared_ptr<Core::GameObject> shared = std::make_shared<Core::GameObject>(std::forward<Args>(args)...);
		shared->m_scene = this;
		shared->Initialize();

		AddObject(shared);

		m_lastAdded.push_back(shared);

		return shared;
	}

	inline void Resource::Scene::AddObject(const std::shared_ptr<Core::GameObject>& gameObject)
	{
		if (!m_objectList.contains(gameObject->m_UUID))
		{
			m_objectList[gameObject->m_UUID] = gameObject;
		}
		else
		{
			ASSERT(false);
		}
	}

	inline void Resource::Scene::RemoveObject(Core::GameObject* object)
	{
		const auto shared = std::ranges::find_if(m_objectList, [&](const std::pair<uint64_t, std::shared_ptr<Core::GameObject>>& element) {
			return element.second.get() == object; });
		if (shared != m_objectList.end()) {
			if (shared != m_objectList.end())
			{
				m_objectList.erase(shared);
			}
		}
	}

	inline void Resource::Scene::RevertObject(size_t number)
	{
		for (size_t i = 0; i < number; ++i)
		{
			auto last = m_lastAdded.back();
			last.lock()->Destroy();
			m_lastAdded.pop_back();
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

	inline Shared<Render::EditorCamera> Resource::Scene::GetEditorCamera() const
	{
		return m_editorCamera;
	}

	inline Weak<Render::Camera> Resource::Scene::GetCurrentCamera() const
	{
		return m_currentCamera;
	}

	inline Shared<Editor::Gizmo> Resource::Scene::GetGizmo() const
	{
		return m_gizmo;
	}

	inline Shared<Editor::ActionManager> Resource::Scene::GetActionManager() const
	{
		return m_actionManager;
	}

	inline const UMap<Core::UUID, Shared<Core::GameObject>>& Resource::Scene::GetObjectList() const
	{
		return m_objectList;
	}
}
