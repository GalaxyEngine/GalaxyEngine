#include "pch.h"
#include "Render/LightManager.h"
#include "Render/Camera.h"
#include "Render/EditorCamera.h"

#include "Resource/Shader.h"

#include "Component/Light.h"

/*
* TODO:
*	Check dirty
*	Check normal in shader
*	SpotLight
*	Bill board shader
*	Draw icon in editor
*/

namespace GALAXY
{
	Unique<Render::LightManager> Render::LightManager::m_instance;
	bool Render::LightManager::AddLight(Weak<Component::Light> light)
	{
		Shared<Component::Light> lightShared = light.lock();
		// Check if inside the list
		size_t freeIndex = -1;

		Component::Light::Type type = lightShared->GetLightType();
		size_t startIndex = (size_t)type * MAX_LIGHT_NUMBER;
		for (size_t i = startIndex; i < startIndex + MAX_LIGHT_NUMBER; i++)
		{
			if (freeIndex == -1 && m_instance->m_lights[i].lock() == nullptr)
			{
				freeIndex = i - startIndex;
			}
			else if (m_instance->m_lights[i].lock() == lightShared)
			{
				return false;
			}
		}
		if (freeIndex == -1)
			return false;
		m_instance->m_lights[startIndex + freeIndex] = light;
		lightShared->SetIndex(freeIndex);

		return true;
	}

	void Render::LightManager::RemoveLight(Weak<Component::Light> light)
	{
		Shared<Component::Light> lightShared = light.lock();
		if (lightShared->GetIndex() == -1)
			return;

		Component::Light::Type type = lightShared->GetLightType();
		size_t startIndex = (size_t)type * MAX_LIGHT_NUMBER;
		size_t indexInArray = startIndex + lightShared->GetIndex();

		ResetLightData(lightShared.get());
		if (m_instance->m_lights[indexInArray].lock() == lightShared)
		{
			m_instance->m_lights[indexInArray].reset();
		}
		lightShared->SetIndex(-1);
	}

	void Render::LightManager::AddShader(Weak<Resource::Shader> shader)
	{
		auto lockShader = shader.lock();
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

	void Render::LightManager::RemoveShader(Weak<Resource::Shader> shader)
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

	void Render::LightManager::SendLightData()
	{
		Vec3f viewPos = Render::Camera::GetEditorCamera()->GetTransform()->GetLocalPosition();
		if (m_lights.empty())
		{
			return;
		}
		for (Weak<Resource::Shader> shader : m_shaders)
		{
			auto lockShader = shader.lock();
			if (!lockShader)
			{
				RemoveShader(shader);
				continue;
			}
			if (!lockShader->HasBeenSent())
				continue;

			lockShader->Use();

			lockShader->SendVec3f("camera.viewPos", viewPos);

			for (Weak<Component::Light> light : m_lights)
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
