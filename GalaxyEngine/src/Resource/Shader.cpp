#include "pch.h"
#include "Resource/Shader.h"
#include "Resource/ResourceManager.h"

#include "Render/LightManager.h"

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
			const Weak<Shader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::Shader>(p_fileInfo.GetFullPath());

			// Parse .shader file
			std::string line;
			while (std::getline(file, line))
			{
				if (line[0] == 'V')
				{
					std::filesystem::path vertPath = line.substr(4);
					vertPath = p_fileInfo.GetFullPath().parent_path() / vertPath;
					Weak<VertexShader> vertexShader = ResourceManager::GetOrLoad<Resource::VertexShader>(vertPath);
					SetVertex(vertexShader.lock(), thisShader);

					m_pickingVariant = Create(vertPath, PICKING_PATH);
					if (m_pickingVariant.lock())
						m_pickingVariant.lock()->ShouldBeDisplayOnInspector(false);
				}
				else if (line[0] == 'G')
				{
					std::filesystem::path geomPath = line.substr(4);
					geomPath = p_fileInfo.GetFullPath().parent_path() / geomPath;
					Weak<GeometryShader> geometryShader = ResourceManager::GetOrLoad<Resource::GeometryShader>(geomPath);
					SetGeometry(geometryShader.lock(), thisShader);
				}
				else if (line[0] == 'F')
				{
					std::filesystem::path fragPath = line.substr(4);
					fragPath = GetFileInfo().GetFullPath().parent_path() / fragPath;
					Weak<FragmentShader> fragmentShader = ResourceManager::GetOrLoad<Resource::FragmentShader>(fragPath);
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
		p_locations.clear();

		p_hasBeenSent = Wrapper::Renderer::GetInstance()->LinkShaders(this);

		if (p_hasBeenSent) {
			const auto weak_this = Resource::ResourceManager::GetOrLoad<Resource::Shader>(GetFileInfo().GetFullPath());
			Render::LightManager::AddShader(weak_this);
		}
	}

	void Resource::Shader::SetVertex(const Shared<VertexShader>& vertexShader, const Weak<Shader>& weak_this)
	{
		ASSERT(vertexShader != nullptr);
		std::get<0>(p_subShaders) = vertexShader;
		vertexShader->AddShader(weak_this);
	}

	void Resource::Shader::SetFragment(const Shared<FragmentShader>& fragmentShader, const Weak<Shader>& weak_this)
	{
		ASSERT(fragmentShader != nullptr);
		std::get<2>(p_subShaders) = fragmentShader;
		fragmentShader->AddShader(weak_this);
	}

	void Resource::Shader::SetGeometry(const Shared<GeometryShader>& geometryShader, const Weak<Shader>& weak_this)
	{
		ASSERT(geometryShader != nullptr);
		std::get<1>(p_subShaders) = geometryShader;
		geometryShader->AddShader(weak_this);
	}

	void Resource::Shader::Recompile() const
	{
		PrintLog("Recompile shader: %s", p_fileInfo.GetFullPath().string().c_str());
		if (const Shared<VertexShader> vertShader = std::get<0>(p_subShaders).lock())
		{
			vertShader->Recompile();
		}
		if (const Shared<GeometryShader> geomShader = std::get<1>(p_subShaders).lock())
		{
			geomShader->Recompile();
		}
		if (const Shared<FragmentShader> fragShader = std::get<2>(p_subShaders).lock())
		{
			fragShader->Recompile();
		}
	}

	Weak<Resource::Shader> Resource::Shader::Create(const std::filesystem::path& vertPath, const std::filesystem::path& fragPath)
	{
		if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(fragPath))
			return {};
		const Weak<VertexShader> vertexShader = Resource::ResourceManager::GetOrLoad<VertexShader>(vertPath);
		const Weak<FragmentShader> fragShader = Resource::ResourceManager::GetOrLoad<FragmentShader>(fragPath);

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
	void Resource::BaseShader::AddShader(const Weak<Shader>& shader)
	{
		if (!shader.lock())
			return;
		const size_t size = p_shaders.size();
		if (size > 0)
		{
			for (auto&& _shader : p_shaders)
			{
				if (!_shader.lock())
					continue;
				if (_shader.lock() == shader.lock())
					return;
			}
		}

		p_shaders.push_back(shader.lock());
	}

	void Resource::BaseShader::Recompile()
	{
		p_shouldBeLoaded = false;
		p_loaded = false;
		p_hasBeenSent = false;
		p_content = "";

		Core::ThreadManager::GetInstance()->AddTask([this] { Load(); });

		for (const Weak<Shader>& shader : p_shaders)
		{
			shader.lock()->p_hasBeenSent = false;
			shader.lock()->SendRequest();
		}
	}

	void Resource::BaseShader::Load()
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

	Resource::FragmentShader::~FragmentShader()
	{
		auto weak_this = Resource::ResourceManager::GetInstance()->GetResource<FragmentShader>(this->GetFileInfo().GetFullPath());
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

	int Resource::Shader::GetLocation(const char* locationName)
	{
		ASSERT(HasBeenSent());
		const auto it = p_locations.find(locationName);
		if (it != p_locations.end())
		{
			return it->second;
		}
		else
			return p_locations[locationName] = p_renderer->GetShaderLocation(p_id, locationName);
	}

	void Resource::Shader::SendInt(const char* locationName, const int value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendInt(locationID, value);
	}

	void Resource::Shader::SendFloat(const char* locationName, const float value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendFloat(locationID, value);
	}

	void Resource::Shader::SendDouble(const char* locationName, const double value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendDouble(locationID, value);
	}

	void Resource::Shader::SendVec2f(const char* locationName, const Vec2f& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendVec2f(locationID, value);
	}

	void Resource::Shader::SendVec3f(const char* locationName, const Vec3f& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendVec3f(locationID, value);
	}

	void Resource::Shader::SendVec4f(const char* locationName, const Vec4f& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendVec4f(locationID, value);
	}

	void Resource::Shader::SendVec2i(const char* locationName, const Vec2i& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendVec2i(locationID, value);
	}

	void Resource::Shader::SendVec3i(const char* locationName, const Vec3i& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendVec3i(locationID, value);
	}

	void Resource::Shader::SendVec4i(const char* locationName, const Vec4i& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendVec4i(locationID, value);
	}

	void Resource::Shader::SendMat4(const char* locationName, const Mat4& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		p_renderer->ShaderSendMat4(locationID, value);
	}

}