#include "pch.h"
#include "Render/LightManager.h"
#include "Render/Camera.h"
#include "Render/EditorCamera.h"

#include "Resource/Shader.h"

#include "Component/Light.h"
namespace GALAXY
{
	Unique<Render::LightManager> Render::LightManager::m_instance;
	void Render::LightManager::AddLight(Weak<Component::Light> light)
	{
		for (auto& lightArray : m_instance->m_lights)
		{
			if (lightArray.lock() == light.lock())
			{
				return;
			}
		}
		m_instance->m_lights.push_back(light);
	}

	void Render::LightManager::RemoveLight(Weak<Component::Light> light)
	{
		for (size_t i = 0; i < m_instance->m_lights.size(); i++)
		{
			if (m_instance->m_lights[i].lock() == light.lock())
			{
				m_instance->m_lights.erase(m_instance->m_lights.begin() + i);
				break;
			}
		}
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
		for (auto& shader : m_shaders)
		{
			auto lockShader = shader.lock();
			if (!lockShader)
			{
				RemoveShader(shader);
				continue;
			}
			lockShader->Use();
			lockShader->SendVec3f("camera.viewPos", viewPos);
			for (auto& light : m_lights)
			{
				if (light.expired())
				{
					RemoveLight(light);
					continue;
				}
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
