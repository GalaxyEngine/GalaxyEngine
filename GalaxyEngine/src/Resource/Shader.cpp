#include "pch.h"
#include "Resource/Shader.h"
#include "Resource/ResourceManager.h"
#define PICKING_PATH ENGINE_RESOURCE_FOLDER_NAME"/shaders/PickingShader/picking.frag"
namespace GALAXY
{
	void Resource::Shader::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		p_renderer = Wrapper::Renderer::GetInstance();
		if (std::get<0>(p_subShaders).lock() || std::get<1>(p_subShaders).lock() || std::get<2>(p_subShaders).lock())
		{
			SendRequest();
			return;
		}
		if (std::fstream file = Utils::FileSystem::OpenFile(p_fileInfo.GetFullPath()); file.is_open())
		{
			Weak<Shader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::Shader>(p_fileInfo.GetFullPath());

			// Parse .shader file
			std::string line;
			while (std::getline(file, line))
			{
				if (line[0] == 'V')
				{
					std::filesystem::path vertPath = line.substr(4);
					vertPath = p_fileInfo.GetFullPath().parent_path() / vertPath;
					Weak<VertexShader> vertexShader = ResourceManager::GetInstance()->GetOrLoad<Resource::VertexShader>(vertPath);
					SetVertex(vertexShader.lock(), thisShader);

					m_pickingVariant = Create(vertPath, PICKING_PATH);
					if (m_pickingVariant.lock())
						m_pickingVariant.lock()->ShouldBeDisplayOnInspector(false);
				}
				else if (line[0] == 'G')
				{
					std::filesystem::path geomPath = line.substr(4);
					geomPath = p_fileInfo.GetFullPath().parent_path() / geomPath;
					Weak<GeometryShader> geometryShader = ResourceManager::GetInstance()->GetOrLoad<Resource::GeometryShader>(geomPath);
					SetGeometry(geometryShader.lock(), thisShader);
				}
				else if (line[0] == 'F')
				{
					std::filesystem::path fragPath = line.substr(4);
					fragPath = p_fileInfo.GetFullPath().parent_path() / fragPath;
					Weak<FragmentShader> fragmentShader = ResourceManager::GetInstance()->GetOrLoad<Resource::FragmentShader>(fragPath);
					SetFragment(fragmentShader.lock(), thisShader);
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

	void Resource::Shader::SetVertex(Shared<VertexShader> vertexShader, Weak<Shader> weak_this)
	{
		ASSERT(vertexShader != nullptr);
		std::get<0>(p_subShaders) = vertexShader;
		vertexShader->AddShader(weak_this);
	}

	void Resource::Shader::SetFragment(Shared<FragmentShader> fragmentShader, Weak<Shader> weak_this)
	{
		ASSERT(fragmentShader != nullptr);
		std::get<2>(p_subShaders) = fragmentShader;
		fragmentShader->AddShader(weak_this);
	}

	void Resource::Shader::SetGeometry(Shared<GeometryShader> geometryShader, Weak<Shader> weak_this)
	{
		ASSERT(geometryShader != nullptr);
		std::get<1>(p_subShaders) = geometryShader;
		geometryShader->AddShader(weak_this);
	}

	Weak<Resource::Shader> Resource::Shader::Create(const std::filesystem::path& vertPath, const std::filesystem::path& fragPath)
	{
		if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(fragPath))
			return Weak<Resource::Shader>();
		Weak<VertexShader> vertexShader = Resource::ResourceManager::GetOrLoad<VertexShader>(vertPath);
		Weak<FragmentShader> fragShader = Resource::ResourceManager::GetOrLoad<FragmentShader>(fragPath);

		// temporary add this to check if it's work to not expire the shader
		Shared<VertexShader> LockVertex = vertexShader.lock();
		Shared<FragmentShader> LockFrag = fragShader.lock();

		std::string shaderPath = vertexShader.lock()->GetFileInfo().GetRelativePath().string() + " + " + fragShader.lock()->GetFileInfo().GetRelativePath().string();

		// Add shader before because of mono thread
		shaderPath = shaderPath + ".shader";
		auto shader = Resource::ResourceManager::GetInstance()->AddResource<Shader>(shaderPath);
		shader.lock()->SetFragment(fragShader.lock(), shader);
		shader.lock()->SetVertex(vertexShader.lock(), shader);

		Resource::ResourceManager::GetOrLoad<Shader>(shaderPath);
		return shader;
	}

	// === Base Shader === //
	void Resource::BaseShader::AddShader(Weak<Shader> shader)
	{
		if (!shader.lock())
			return;
		size_t size = p_shader.size();
		if (size > 0)
		{
			for (auto&& _shader : p_shader)
			{
				if (!_shader.lock())
					continue;
				if (_shader.lock() == shader.lock())
					return;
			}
		}

		p_shader.push_back(shader.lock());
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
		p_renderer->UseShader(this);
	}

	int Resource::Shader::GetLocation(const std::string& locationName)
	{
		ASSERT(HasBeenSent());
		if (p_locations.count(locationName))
		{
			return p_locations.at(locationName);
		}
		else
			return p_locations[locationName] = p_renderer->GetShaderLocation(p_id, locationName);
	}

	void Resource::Shader::SendInt(const std::string& locationName, int value)
	{
		p_renderer->ShaderSendInt(GetLocation(locationName), value);
	}

	void Resource::Shader::SendFloat(const std::string& locationName, float value)
	{
		p_renderer->ShaderSendFloat(GetLocation(locationName), value);
	}

	void Resource::Shader::SendDouble(const std::string& locationName, double value)
	{
		p_renderer->ShaderSendDouble(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec2f(const std::string& locationName, const Vec2f& value)
	{
		p_renderer->ShaderSendVec2f(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec3f(const std::string& locationName, const Vec3f& value)
	{
		p_renderer->ShaderSendVec3f(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec4f(const std::string& locationName, const Vec4f& value)
	{
		p_renderer->ShaderSendVec4f(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec2i(const std::string& locationName, const Vec2i& value)
	{
		p_renderer->ShaderSendVec2i(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec3i(const std::string& locationName, const Vec3i& value)
	{
		p_renderer->ShaderSendVec3i(GetLocation(locationName), value);
	}

	void Resource::Shader::SendVec4i(const std::string& locationName, const Vec4i& value)
	{
		p_renderer->ShaderSendVec4i(GetLocation(locationName), value);
	}

	void Resource::Shader::SendMat4(const std::string& locationName, const Mat4& value)
	{
		p_renderer->ShaderSendMat4(GetLocation(locationName), value);
	}

}