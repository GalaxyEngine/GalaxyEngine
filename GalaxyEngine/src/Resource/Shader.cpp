#include "pch.h"
#include "Resource/Shader.h"
#include "Resource/ResourceManager.h"
#define PICKING_PATH ENGINE_RESOURCE_FOLDER_NAME"\\shaders\\PickingShader\\picking.frag"
namespace GALAXY {
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
			std::weak_ptr<Shader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::Shader>(p_fileInfo.GetFullPath());

			// Parse .shader file
			std::string line;
			while (std::getline(file, line)) {
				if (line[0] == 'V')
				{
					std::filesystem::path vertPath = line.substr(4);
					vertPath = p_fileInfo.GetFullPath().parent_path() / vertPath;
					std::weak_ptr<VertexShader> vertexShader = ResourceManager::GetInstance()->GetOrLoad<Resource::VertexShader>(vertPath);
					SetVertex(vertexShader, thisShader);

					m_pickingVariant = Create(vertPath, PICKING_PATH);
					if (m_pickingVariant.lock())
						m_pickingVariant.lock()->ShouldBeDisplayOnInspector(false);
				}
				else if (line[0] == 'G')
				{
					std::filesystem::path geomPath = line.substr(4);
					geomPath = p_fileInfo.GetFullPath().parent_path() / geomPath;
					std::weak_ptr<GeometryShader> geometryShader = ResourceManager::GetInstance()->GetOrLoad<Resource::GeometryShader>(geomPath);
					SetGeometry(geometryShader, thisShader);
				}
				else if (line[0] == 'F')
				{
					std::filesystem::path fragPath = line.substr(4);
					fragPath = p_fileInfo.GetFullPath().parent_path() / fragPath;
					std::weak_ptr<FragmentShader> fragmentShader = ResourceManager::GetInstance()->GetOrLoad<Resource::FragmentShader>(fragPath);
					SetFragment(fragmentShader, thisShader);
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

	void Resource::Shader::SetVertex(Weak<VertexShader> vertexShader, Weak<Shader> weak_this)
	{
		std::get<0>(p_subShaders) = vertexShader;
		vertexShader.lock()->AddShader(weak_this);
	}

	void Resource::Shader::SetFragment(Weak<FragmentShader> fragmentShader, Weak<Shader> weak_this)
	{
		std::get<2>(p_subShaders) = fragmentShader;
		fragmentShader.lock()->AddShader(weak_this);
	}

	void Resource::Shader::SetGeometry(Weak<GeometryShader> geometryShader, Weak<Shader> weak_this)
	{
		std::get<1>(p_subShaders) = geometryShader;
		geometryShader.lock()->AddShader(weak_this);
	}

	Weak<Resource::Shader> Resource::Shader::Create(const std::filesystem::path& vertPath, const std::filesystem::path& fragPath)
	{
		if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(fragPath))
			return Weak<Resource::Shader>();
		auto vertexShader = Resource::ResourceManager::GetOrLoad<VertexShader>(vertPath);
		auto fragShader = Resource::ResourceManager::GetOrLoad<FragmentShader>(fragPath);
		std::string shaderPath = vertexShader.lock()->GetFileInfo().GetRelativePath().string() + " + " + fragShader.lock()->GetFileInfo().GetRelativePath().string();

		// Add shader before because of mono thread
		shaderPath = shaderPath + ".shader";
		auto shader = Resource::ResourceManager::GetInstance()->AddResource<Shader>(shaderPath);
		shader.lock()->SetFragment(fragShader, shader);
		shader.lock()->SetVertex(vertexShader, shader);

		Resource::ResourceManager::GetOrLoad<Shader>(shaderPath);
		return shader;
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