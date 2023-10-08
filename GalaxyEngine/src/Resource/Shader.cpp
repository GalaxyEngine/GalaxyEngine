#include "pch.h"
#include "Resource/Shader.h"
#include "Resource/ResourceManager.h"

namespace GALAXY {
	void Resource::Shader::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		m_renderer = Wrapper::Renderer::GetInstance();
		if (std::fstream file = Utils::FileSystem::OpenFile(p_fileInfo.GetFullPath()); file.is_open())
		{
			// Parse .shader file
			std::string line;
			while (std::getline(file, line)) {
				if (line[0] == 'V')
				{
					std::filesystem::path vertPath = line.substr(4);
					vertPath = p_fileInfo.GetFullPath().parent_path() / vertPath;
					std::weak_ptr<VertexShader> vertexShader = ResourceManager::GetInstance()->GetOrLoad<Resource::VertexShader>(vertPath);
					std::get<0>(m_subShaders) = vertexShader;
					std::weak_ptr<Shader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::Shader>(p_fileInfo.GetFullPath());
					vertexShader.lock()->AddShader(thisShader);
				}
				else if (line[0] == 'G')
				{
					std::filesystem::path geomPath = line.substr(4);
					geomPath = p_fileInfo.GetFullPath().parent_path() / geomPath;
					std::weak_ptr<GeometryShader> geometryShader = ResourceManager::GetInstance()->GetOrLoad<Resource::GeometryShader>(geomPath);
					std::get<1>(m_subShaders) = geometryShader;
					std::weak_ptr<Shader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::Shader>(p_fileInfo.GetFullPath());
					geometryShader.lock()->AddShader(thisShader);
				}
				else if (line[0] == 'F')
				{
					std::filesystem::path fragPath = line.substr(4);
					fragPath = p_fileInfo.GetFullPath().parent_path() / fragPath;
					std::weak_ptr<FragmentShader> fragmentShader = ResourceManager::GetInstance()->GetOrLoad<Resource::FragmentShader>(fragPath);
					std::get<2>(m_subShaders) = fragmentShader;
					std::weak_ptr<Shader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::Shader>(p_fileInfo.GetFullPath());
					fragmentShader.lock()->AddShader(thisShader);
				}
			}
			SendRequest();
		}
	}

	void Resource::Shader::Send()
	{
		if (p_hasBeenSent)
			return;
		p_hasBeenSent = Wrapper::Renderer::GetInstance()->LinkShaders(this);
	}

	// === Base Shader === //
	void Resource::BaseShader::AddShader(std::weak_ptr<Shader> shader)
	{
		if (!shader.lock())
			return;
		uint64_t count = std::count_if(p_shader.begin(), p_shader.end(), [&shader](const std::weak_ptr<Shader>& wp) {
			return !wp.expired() && !wp.owner_before(shader) && !shader.owner_before(wp);
			});
		if (count == 0)
		{
			p_shader.push_back(shader);
		}
	}

	void GALAXY::Resource::BaseShader::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		p_content = Utils::FileSystem::ReadFile(p_fileInfo.GetFullPath());
		p_loaded = true;
		SendRequest();
	}


	// === Vertex === //
	void Resource::VertexShader::Send()
	{
		if (p_hasBeenSent)
			return;
		p_hasBeenSent = Wrapper::Renderer::GetInstance()->CompileVertexShader(this);
	}

	// === Fragment === //
	void Resource::FragmentShader::Send()
	{
		if (p_hasBeenSent)
			return;
		p_hasBeenSent = Wrapper::Renderer::GetInstance()->CompileFragmentShader(this);
	}

	void Resource::Shader::Use()
	{
		m_renderer->UseShader(this);
	}

	int Resource::Shader::GetLocation(const std::string& locationName)
	{
		if (m_locations.count(locationName))
		{
			return m_locations.at(locationName);
		}
		else
			return m_locations[locationName] = m_renderer->GetShaderLocation(m_id, locationName);
	}

	void Resource::Shader::SendInt(const std::string& locationName, int value)
	{
		m_renderer->ShaderSendInt(GetLocation(locationName), value);
	}

	void Resource::Shader::SendFloat(const std::string& locationName, float value)
	{
		m_renderer->ShaderSendFloat(GetLocation(locationName), value);
	}

	void Resource::Shader::SendDouble(const std::string& locationName, double value)
	{
		m_renderer->ShaderSendDouble(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec2f(const std::string& locationName, const Vec2f& value)
	{
		m_renderer->ShaderSendVec2f(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec3f(const std::string& locationName, const Vec3f& value)
	{
		m_renderer->ShaderSendVec3f(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec4f(const std::string& locationName, const Vec4f& value)
	{
		m_renderer->ShaderSendVec4f(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec2i(const std::string& locationName, const Vec2i& value)
	{
		m_renderer->ShaderSendVec2i(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec3i(const std::string& locationName, const Vec3i& value)
	{
		m_renderer->ShaderSendVec3i(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec4i(const std::string& locationName, const Vec4i& value)
	{
		m_renderer->ShaderSendVec4i(GetLocation(locationName), value);
	}

	void Resource::Shader::SendMat4(const std::string& locationName, const Mat4& value)
	{
		m_renderer->ShaderSendMat4(GetLocation(locationName), value);
	}

}