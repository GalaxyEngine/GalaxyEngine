#include "pch.h"
#include "Resource/Shader.h"
#include "Resource/ResourceManager.h"

#include "Render/LightManager.h"

namespace GALAXY
{

#pragma region Content Shader
	const char* vertShaderContent = R"(#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNor;
layout (location = 3) in vec3 aTan;

out vec3 pos;  
out vec2 uv;
out vec3 normal;
out vec3 tangent;

uniform mat4 MVP;
uniform mat4 Model;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0f);
    pos = vec3(Model * vec4(aPos, 1.0f)); 
	normal = mat3(transpose(inverse(Model))) * aNor;
    tangent = mat3(Model) * aTan;
    uv = aTex;
})";

	const char* fragShaderContent =
		R"(#version 450 core

const int LightNumber = 8;

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    sampler2D albedo;
    bool hasAlbedo;
    sampler2D normalMap;
    bool hasNormalMap;
};

struct DirectionalLight {
    bool enable;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 direction;
};

struct PointLight {
    bool enable;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    vec3 position;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    bool enable;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    vec3 position;
    float constant;
    float linear;
    float quadratic;

    vec3 direction;
    float cutOff;
    float outerCutOff;
};

struct Camera
{
    vec3 viewPos;
};

out vec4 FragColor;

in vec3 pos;
in vec2 uv;
in vec3 normal;
in vec3 tangent;

uniform Material material;
uniform DirectionalLight directionals[LightNumber];
uniform PointLight points[LightNumber];
uniform SpotLight spots[LightNumber];
uniform Camera camera;

uniform bool UseLights;

void main()
{
    if (material.hasAlbedo)
        FragColor = texture(material.albedo, uv);
    else
        FragColor = material.diffuse;
} )";

	const char* shaderContent =
		R"( ------------- Shader ------------- 
[Vertex] : %llu
[Geometry] : -1
[Fragment] : %llu
 ============= Shader ============= 

)";
#pragma endregion

	void Resource::Shader::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		
		if (std::get<0>(p_subShaders).lock() || std::get<1>(p_subShaders).lock() || std::get<2>(p_subShaders).lock())
		{
			p_loaded = true;
			SendRequest();
			return;
		}
		StartLoading();

		const Weak<Shader> thisShader = ResourceManager::GetResource<Resource::Shader>(p_fileInfo.GetFullPath());

		CppSer::Parser parser(GetFileInfo().GetFullPath());
		if (parser.IsFileOpen())
		{
			Core::UUID vertexUUID = parser["Vertex"].As<uint64_t>();
			if (vertexUUID != UUID_NULL)
			{
				Weak<VertexShader> vertexShader = ResourceManager::GetOrLoad<Resource::VertexShader>(vertexUUID);
				ASSERT(vertexShader.lock());
				if (vertexShader.lock()) {
					SetVertex(vertexShader.lock(), thisShader);
				}
			}
			Core::UUID geometryUUID = parser["Geometry"].As<uint64_t>();
			if (geometryUUID != UUID_NULL)
			{
				Weak<GeometryShader> geometryShader = ResourceManager::GetOrLoad<Resource::GeometryShader>(geometryUUID);
				SetGeometry(geometryShader.lock(), thisShader);
			}
			Core::UUID fragmentUUID = parser["Fragment"].As<uint64_t>();
			if (fragmentUUID != UUID_NULL)
			{
				Weak<FragmentShader> fragmentShader = ResourceManager::GetOrLoad<Resource::FragmentShader>(fragmentUUID);
				ASSERT(fragmentShader.lock());
				SetFragment(fragmentShader.lock(), thisShader);
			}
		}
		p_loaded = true;
		SendRequest();
	}
	
	void Resource::Shader::Send()
	{
		if (p_hasBeenSent)
			return;
		p_hasBeenSent = true;

		p_hasBeenSent = Wrapper::Renderer::GetInstance()->LinkShaders(this);

		p_uniforms.clear();

		p_uniforms = Wrapper::Renderer::GetInstance()->GetShaderUniforms(this);

		if (p_hasBeenSent && !p_isAVariant) {
			const auto weak_this = Resource::ResourceManager::GetResource<Resource::Shader>(GetFileInfo().GetFullPath());
			Render::LightManager::AddShader(weak_this);
			if (!std::filesystem::exists(GetDataFilePath()))
				CreateDataFile();
			FinishLoading();
		}
		if (p_hasBeenSent.load())
			OnLoad.Invoke();
	}

	void Resource::Shader::Save()
	{
		// Debug
		ASSERT(GetVertex().lock() || GetFragment().lock());

		CppSer::Serializer serializer(p_fileInfo.GetFullPath());
		serializer <<CppSer::Pair::BeginMap << "Shader";
		SerializeResource(serializer, "Vertex", GetVertex());
		SerializeResource(serializer, "Geometry", GetGeometry());
		SerializeResource(serializer, "Fragment", GetFragment());
		serializer <<CppSer::Pair::EndMap << "Shader";
	}

#ifdef WITH_EDITOR
	Path Resource::Shader::GetThumbnailPath() const
	{
		return SHADER_ICON_PATH;
	}
#endif

	void Resource::Shader::ShowInInspector()
	{
#ifdef WITH_EDITOR
		// Vertex Shader
		ImGui::TextUnformatted("Vertex Shader");
		ImGui::SameLine();
		Shared<VertexShader> vertex = GetVertex().lock();
		Vec2f buttonSize = { ImGui::GetContentRegionAvail().x, 0 };
		if (ImGui::Button(vertex ? vertex->GetFileInfo().GetFileName().c_str() : "Empty##0", buttonSize))
		{
			ImGui::OpenPopup("VertexPopup");
		}
		Weak<VertexShader> vertexShader;
		if (ResourceManager::ResourcePopup("VertexPopup", vertexShader))
		{
			SetVertex(vertexShader.lock(), ResourceManager::GetResource<Shader>(this->GetFileInfo().GetFullPath()));
			Recompile();
		}

		// Geometry Shader
		ImGui::TextUnformatted("Geometry Shader");
		ImGui::SameLine();
		Shared<GeometryShader> geometry = GetGeometry().lock();
		buttonSize = { ImGui::GetContentRegionAvail().x, 0 };
		if (ImGui::Button(geometry ? GetVertex().lock()->GetFileInfo().GetFileName().c_str() : "Empty##1", buttonSize))
		{
			ImGui::OpenPopup("GeometryPopup");
		}
		Weak<GeometryShader> geometryShader;
		if (ResourceManager::ResourcePopup("GeometryPopup", geometryShader))
		{
			SetGeometry(geometryShader.lock(), ResourceManager::GetResource<Shader>(this->GetFileInfo().GetFullPath()));
			Recompile();
		}

		// Fragment Shader
		ImGui::TextUnformatted("Fragment Shader");
		ImGui::SameLine();
		const Shared<FragmentShader> frag = GetFragment().lock();
		buttonSize = { ImGui::GetContentRegionAvail().x, 0 };
		if (ImGui::Button(frag ? frag->GetFileInfo().GetFileName().c_str() : "Empty##2", buttonSize))
		{
			ImGui::OpenPopup("FragmentPopup");
		}
		Weak<FragmentShader> fragmentShader;
		if (ResourceManager::ResourcePopup("FragmentPopup", fragmentShader))
		{
			SetFragment(fragmentShader.lock(), ResourceManager::GetResource<Shader>(this->GetFileInfo().GetFullPath()));
			Recompile();
		}

		if (ImGui::Button("Save"))
		{
			Save();
		}
#endif
	}

	void Resource::Shader::SetVertex(const Shared<VertexShader>& vertexShader, const Weak<Shader>& weak_this, bool createVariant /*= true*/)
	{
		Shared<BaseShader> shader = std::get<0>(p_subShaders).lock();
		if (shader)
			shader->RemoveShader(this);

		std::get<0>(p_subShaders) = vertexShader;
		if (vertexShader)
			vertexShader->AddShader(weak_this);

		if (!createVariant)
			return;
		m_pickingVariant = Create(vertexShader->GetFileInfo().GetFullPath(), PICKING_PATH);
		if (m_pickingVariant)
		{
			m_pickingVariant->SetDisplayOnInspector(false);
			m_pickingVariant->SetCreateDataFile(false);
		}
	}

	void Resource::Shader::SetGeometry(const Shared<GeometryShader>& geometryShader, const Weak<Shader>& weak_this)
	{
		Shared<BaseShader> shader = std::get<1>(p_subShaders).lock();
		if (shader)
			shader->RemoveShader(this);

		std::get<1>(p_subShaders) = geometryShader;
		if (geometryShader)
			geometryShader->AddShader(weak_this);
	}

	void Resource::Shader::SetFragment(const Shared<FragmentShader>& fragmentShader, const Weak<Shader>& weak_this)
	{
		Shared<BaseShader> shader = std::get<2>(p_subShaders).lock();
		if (shader)
		{
			shader->RemoveShader(this);
		}
		std::get<2>(p_subShaders) = fragmentShader;
		if (fragmentShader)
			fragmentShader->AddShader(weak_this);
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


	Shared<Resource::Shader> Resource::Shader::Create(const Path& vertPath, const Path& fragPath)
	{
		if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(fragPath))
			return {};
		const Weak<VertexShader> vertexShader = ResourceManager::GetOrLoad<VertexShader>(vertPath);
		const Weak<FragmentShader> fragShader = ResourceManager::GetOrLoad<FragmentShader>(fragPath);

		// temporary add this to check if it's work to not expire the shader
		Shared<VertexShader> LockVertex = vertexShader.lock();
		Shared<FragmentShader> LockFrag = fragShader.lock();

		std::string shaderPath = vertexShader.lock()->GetFileInfo().GetFullPath().string() + " + " + fragShader.lock()->GetFileInfo().GetRelativePath().string();

		// Add shader before because of mono thread
		shaderPath = shaderPath + ".shader";
		auto shader = ResourceManager::TemporaryAdd<Shader>(shaderPath);
		shader->p_isAVariant = true;
		shader->SetFragment(fragShader.lock(), shader);
		shader->SetVertex(vertexShader.lock(), shader, false);

		shader = ResourceManager::TemporaryLoad<Shader>(shaderPath);
		return shader;
	}


	Weak<Resource::Shader> Resource::Shader::Create(const Path& path)
	{
		std::filesystem::create_directory(path);
		const auto shaderPath = path / path.filename().stem();

		const std::string vertexPath = shaderPath.string() + ".vert";
		const std::string fragPath = shaderPath.string() + ".frag";

		// Create vertex File
		std::ofstream vertFile(vertexPath);
		if (vertFile.is_open()) {
			vertFile << vertShaderContent;
			vertFile.close();
		}

		// Create fragment File
		std::ofstream fragFile(fragPath);
		if (fragFile.is_open()) {
			fragFile << fragShaderContent;
			fragFile.close();
		}
		auto vertex = ResourceManager::AddResource<VertexShader>(vertexPath).lock();
		vertex->CreateDataFile();
		auto frag = ResourceManager::AddResource<FragmentShader>(fragPath).lock();
		frag->CreateDataFile();

		uint64_t vUUID = vertex->GetUUID();
		uint64_t fUUID = frag->GetUUID();

		// Create shader File
		std::string shaderResourcePath = shaderPath.string() + ".shader";
		std::ofstream shaderFile(shaderResourcePath);
		char content[1024];
		snprintf(content, sizeof(content), shaderContent, vUUID, fUUID);
		if (shaderFile.is_open()) {
			shaderFile << content;
			shaderFile.close();
		}

		return ResourceManager::GetOrLoad<Shader>(shaderResourcePath);
	}

	void Resource::Shader::Serialize(CppSer::Serializer& serializer) const
	{
		IResource::Serialize(serializer);
		/*
		serializer <<CppSer::Pair::BeginMap << "Shader";
		serializer << CppSer::Pair::BeginTab;

		const Shared<VertexShader> vertex = std::get<0>(p_subShaders).lock();
		const Shared<GeometryShader> geometry = std::get<1>(p_subShaders).lock();
		const Shared<FragmentShader> fragment = std::get<2>(p_subShaders).lock();

		ASSERT(vertex != nullptr);
		ASSERT(fragment != nullptr);

		SerializeResource(serializer, "Vertex", std::get<0>(p_subShaders));
		SerializeResource(serializer, "Geometry", std::get<1>(p_subShaders));
		SerializeResource(serializer, "Fragment", std::get<2>(p_subShaders));

		serializer << CppSer::Pair::EndTab;
		serializer <<CppSer::Pair::EndMap << "Shader";
		*/
	}

	void Resource::Shader::Deserialize(CppSer::Parser& parser)
	{
		IResource::Deserialize(parser);
		/*
		parser.PushDepth();

		const Core::UUID vertexUUID = parser["Vertex"].As<uint64_t>();
		const Core::UUID geometryUUID = parser["Geometry"].As<uint64_t>();
		const Core::UUID fragmentUUID = parser["Fragment"].As<uint64_t>();

		std::get<0>(p_subShaders) = ResourceManager::GetResource<VertexShader>(vertexUUID);
		std::get<1>(p_subShaders) = ResourceManager::GetResource<GeometryShader>(geometryUUID);
		std::get<2>(p_subShaders) = ResourceManager::GetResource<FragmentShader>(fragmentUUID);
		*/
	}
	
	void Resource::BaseShader::ShowInInspector()
	{
#ifdef WITH_EDITOR
		size_t i = 0;
		for (auto& shader : p_shaders)
		{
			ImGui::Separator();
			auto shaderPtr = shader.lock();
			if (shaderPtr)
				ImGui::TextWrapped(shaderPtr->GetFileInfo().GetRelativePath().string().c_str());
			else
				ImGui::TextWrapped("Empty");
		}
		ImGui::SeparatorText("Content");
		ImGui::TextWrapped(p_content.c_str());
#endif
	}

	// === Base Shader === //
	void Resource::BaseShader::AddShader(const Weak<Shader>& shader)
	{
		if (!shader.lock())
			return;
		const size_t size = p_shaders.size();
		if (size > 0)
		{
			for (const auto& _shader : p_shaders)
			{
				if (!_shader.lock())
					continue;
				if (_shader.lock() == shader.lock())
					return;
			}
		}

		Core::ThreadManager::Lock();
		p_shaders.push_back(shader.lock());
		Core::ThreadManager::Unlock();
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

	void Resource::BaseShader::RemoveShader(Shader* shader)
	{
		auto new_end = std::remove_if(p_shaders.begin(), p_shaders.end(),
			[&shader](const auto& _shader) {
				return !_shader.expired() && _shader.lock().get() == shader;
			});

		p_shaders.erase(new_end, p_shaders.end());
	}

	void Resource::BaseShader::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		p_content = Utils::FileSystem::ReadFile(p_fileInfo.GetFullPath());
		p_loaded = true;
		
		if (!std::filesystem::exists(GetDataFilePath()))
			CreateDataFile();
		
		// Handle case of reload
		for (const Weak<Shader>& shader : p_shaders)
		{
			if (shader.lock())
			{
				shader.lock()->p_hasBeenSent = false;
				shader.lock()->SendRequest();
			}
		}
		SendRequest();
	}

	void Resource::BaseShader::OnAdd()
	{
		if (!std::filesystem::exists(GetDataFilePath()))
			CreateDataFile();
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
		Wrapper::Renderer::GetInstance()->UseShader(this);
	}

	int Resource::Shader::GetLocation(const char* locationName)
	{
		ASSERT(HasBeenSent());
		const auto it = p_uniforms.find(locationName);
		if (it != p_uniforms.end())
		{
			return it->second.location;
		}
		else
		{
			p_uniforms[locationName] = {};
			return -1;
		}
	}

	void Resource::Shader::SendInt(const char* locationName, const int value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendInt(locationID, value);
	}

	void Resource::Shader::SendFloat(const char* locationName, const float value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendFloat(locationID, value);
	}

	void Resource::Shader::SendDouble(const char* locationName, const double value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendDouble(locationID, value);
	}

	void Resource::Shader::SendVec2f(const char* locationName, const Vec2f& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendVec2f(locationID, value);
	}

	void Resource::Shader::SendVec3f(const char* locationName, const Vec3f& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendVec3f(locationID, value);
	}

	void Resource::Shader::SendVec4f(const char* locationName, const Vec4f& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendVec4f(locationID, value);
	}

	void Resource::Shader::SendVec2i(const char* locationName, const Vec2i& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendVec2i(locationID, value);
	}

	void Resource::Shader::SendVec3i(const char* locationName, const Vec3i& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendVec3i(locationID, value);
	}

	void Resource::Shader::SendVec4i(const char* locationName, const Vec4i& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendVec4i(locationID, value);
	}

	void Resource::Shader::SendMat4(const char* locationName, const Mat4& value)
	{
		const int locationID = GetLocation(locationName);
		if (locationID == -1)
			return;
		Wrapper::Renderer::GetInstance()->ShaderSendMat4(locationID, value);
	}

}