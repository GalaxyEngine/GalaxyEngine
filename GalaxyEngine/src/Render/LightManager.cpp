#include "pch.h"
#include "Render/LightManager.h"
#include "Render/Camera.h"
#include "Render/EditorCamera.h"

#include "Resource/Shader.h"

#include "Component/Light.h"

/*
* TODO:
*	Parallax mapping with normal mapping
*/

namespace GALAXY
{
	Unique<Render::LightManager> Render::LightManager::m_instance;
	bool Render::LightManager::AddLight(const Weak<Component::Light>& light)
	{
		const Shared<Component::Light> lightShared = light.lock();
		// Check if inside the list
		size_t freeIndex = INDEX_NONE;

		Component::Light::Type type = lightShared->GetLightType();
		const size_t startIndex = static_cast<size_t>(type) * MAX_LIGHT_NUMBER;
		for (size_t i = startIndex; i < startIndex + MAX_LIGHT_NUMBER; i++)
		{
			if (freeIndex == INDEX_NONE && m_instance->m_lights[i].lock() == nullptr)
			{
				freeIndex = i - startIndex;
			}
			else if (m_instance->m_lights[i].lock() == lightShared)
			{
				return false;
			}
		}
		if (freeIndex == INDEX_NONE)
			return false;
		m_instance->m_lights[startIndex + freeIndex] = light;
		lightShared->SetLightIndex(freeIndex);

		return true;
	}

	void Render::LightManager::RemoveLight(const Weak<Component::Light>& light)
	{
		const Shared<Component::Light> lightShared = light.lock();
		if (lightShared->GetLightIndex() == INDEX_NONE)
			return;

		Component::Light::Type type = lightShared->GetLightType();
		const size_t startIndex = static_cast<size_t>(type) * MAX_LIGHT_NUMBER;
		const size_t indexInArray = startIndex + lightShared->GetLightIndex();

		ResetLightData(lightShared.get());
		if (m_instance->m_lights[indexInArray].lock() == lightShared)
		{
			m_instance->m_lights[indexInArray].reset();
		}
		lightShared->SetLightIndex(INDEX_NONE);
	}

	void Render::LightManager::AddShader(const Weak<Resource::Shader>& shader)
	{
		const auto lockShader = shader.lock();
		ASSERT(lockShader && lockShader->HasBeenSent());

		if (lockShader->GetLocation("UseLights") == -1)
			return;

		// Check if inside the list
		for (auto& shaderInList : m_instance->m_shaders)
		{
			if (shaderInList.lock() == shader.lock())
				return;
		}
		m_instance->m_shaders.push_back(shader);
	}

	void Render::LightManager::RemoveShader(const Weak<Resource::Shader>& shader)
	{
		for (size_t i = 0; i < m_instance->m_shaders.size(); i++)
		{
			if (m_instance->m_shaders[i].lock() == shader.lock())
			{
				m_instance->m_shaders.erase(m_instance->m_shaders.begin() + i);
				break;
			}
		}
	}

	void Render::LightManager::SendLightData() const
	{
		const Vec3f viewPos = Render::Camera::GetEditorCamera()->GetTransform()->GetLocalPosition();
		if (m_lights.empty())
		{
			return;
		}
		for (const Weak<Resource::Shader>& shader : m_shaders)
		{
			Shared<Resource::Shader> lockShader = shader.lock();
			if (!lockShader)
			{
				RemoveShader(shader);
				continue;
			}
			if (!lockShader->HasBeenSent())
				continue;

			lockShader->Use();

			lockShader->SendVec3f("camera.viewPos", viewPos);

			for (const Weak<Component::Light>& light : m_lights)
			{
				if (!light.lock())
					continue;
				light.lock()->SendLightValues(lockShader.get());
			}
		}
	}

	void Render::LightManager::ResetLightData(Component::Light* light)
	{
		for (auto& shader : m_instance->m_shaders)
		{
			shader.lock()->Use();
			light->ResetLightValues(shader.lock().get());
		}
	}

	Render::LightManager* Render::LightManager::GetInstance()
	{
		if (!m_instance)
		{
			m_instance = std::make_unique<LightManager>();
		}
		return m_instance.get();
	}

}
