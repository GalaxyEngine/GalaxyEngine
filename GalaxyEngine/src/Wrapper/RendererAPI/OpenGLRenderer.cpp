#include "pch.h"
#include "Wrapper/RendererAPI/OpenGLRenderer.h"

#include <glad/glad.h>

#include "Wrapper/Renderer.h"
#include "Wrapper/Window.h"
#include "Wrapper/ImageLoader.h"

#include "Core/SceneHolder.h"

#include "Component/Transform.h"

#include "Render/Framebuffer.h"
#include "Resource/Cubemap.h"

#include "Resource/Scene.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"

#if WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"
#include "Editor/UI/DebugWindow.h"
#endif

namespace GALAXY
{
	// OpenGL Renderer
	void Wrapper::RendererAPI::OpenGLRenderer::Initialize()
	{
		if (!gladLoadGLLoader((GLADloadproc)(Window::GetProcAddress))) {
			PrintError("Failed to initialize GLAD");
			return;
		}

		const GLubyte* glVersion = glGetString(GL_VERSION);
		PrintLog("Loaded OpenGL %s\n", glVersion);
		p_initalized = true;
	}

	void GLAPIENTRY DebugCallback(const GLenum source, const GLenum type, const GLuint id, const GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131154) return;
		if (id == 7) return;// lineWidth
		if (id == 1282) return; // undefined

		std::string log;
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             log += "Source: API\n"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   log += "Source: Window System\n"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: log += "Source: Shader Compiler\n"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     log += "Source: Third Party\n"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     log += "Source: Application\n"; break;
		case GL_DEBUG_SOURCE_OTHER:           log += "Source: Other\n"; break;
		}
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               log += "Type: Error\n"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: log += "Type: Deprecated Behaviour\n"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  log += "Type: Undefined Behaviour\n"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         log += "Type: Portability\n"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         log += "Type: Performance\n"; break;
		case GL_DEBUG_TYPE_MARKER:              log += "Type: Marker\n"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          log += "Type: Push Group\n"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           log += "Type: Pop Group\n"; break;
		case GL_DEBUG_TYPE_OTHER:               log += "Type: Other\n"; break;
		}

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         LOG(Debug::LogType::L_ERROR, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       LOG(Debug::LogType::L_WARNING, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
		case GL_DEBUG_SEVERITY_LOW:          LOG(Debug::LogType::L_WARNING, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: LOG(Debug::LogType::L_INFO, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
		}
	}

	void Wrapper::RendererAPI::OpenGLRenderer::EnableDebugOutput()
	{
		GLint flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(DebugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::UseShader(Resource::Shader* shader)
	{
		glUseProgram(shader->p_id);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::Viewport(const Vec2i& pos, const Vec2i& size)
	{
		glViewport(pos.x, pos.y, size.x, size.y);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ClearColorAndBuffer(const Vec4f& color)
	{
		glClearColor(color.x, color.y, color.z, color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	bool Wrapper::RendererAPI::OpenGLRenderer::LinkShaders(Resource::Shader* shader)
	{
		auto& [vertex, geometry, fragment] = shader->p_subShaders;
		if (!vertex.lock() || !vertex.lock()->HasBeenSent() || !fragment.lock() || !fragment.lock()->HasBeenSent())
			return false;
		// link shaders
		shader->p_id = glCreateProgram();
		glAttachShader(shader->p_id, vertex.lock()->m_id);
		glAttachShader(shader->p_id, fragment.lock()->m_id);
		glLinkProgram(shader->p_id);
		// check for linking errors
		int success;
		glGetProgramiv(shader->p_id, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(shader->p_id, 512, nullptr, infoLog);
			PrintError("Error when Link shader %s :\n %s", shader->GetFileInfo().GetRelativePath().string().c_str(), infoLog);
			return false;
		}
		// PrintLog("Linked shader %s", shader->GetFileInfo().GetRelativePath().string().c_str());
		return true;
	}

	bool Wrapper::RendererAPI::OpenGLRenderer::CompileVertexShader(Resource::VertexShader* vertex)
	{
		// Create a vertex shader object
		vertex->m_id = glCreateShader(GL_VERTEX_SHADER);
		const char* content = vertex->p_content.c_str();
		glShaderSource(vertex->m_id, 1, &content, nullptr);

		// Compile the shader
		glCompileShader(vertex->m_id);

		// Check for compilation errors
		int success;
		glGetShaderiv(vertex->m_id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLint infoLogLength;
			glGetShaderiv(vertex->m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<GLchar> infoLog(infoLogLength);
			glGetShaderInfoLog(vertex->m_id, infoLogLength, nullptr, infoLog.data());

			PrintError("Vertex shader %s compilation failed:\n %s", vertex->GetFileInfo().GetRelativePath().string().c_str(), infoLog.data());
			glDeleteShader(vertex->m_id);
			return false;
		}

		PrintLog("Vertex shader %s compiled", vertex->GetFileInfo().GetRelativePath().string().c_str());
		return true;
	}

	bool Wrapper::RendererAPI::OpenGLRenderer::CompileFragmentShader(Resource::FragmentShader* fragment)
	{
		// Create a vertex shader object
		fragment->m_id = glCreateShader(GL_FRAGMENT_SHADER);
		const char* content = fragment->p_content.c_str();
		glShaderSource(fragment->m_id, 1, &content, nullptr);

		// Compile the shader
		glCompileShader(fragment->m_id);

		// Check for compilation errors
		int success;
		glGetShaderiv(fragment->m_id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLint infoLogLength;
			glGetShaderiv(fragment->m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<GLchar> infoLog(infoLogLength);
			glGetShaderInfoLog(fragment->m_id, infoLogLength, nullptr, infoLog.data());

			PrintError("Fragment shader %s compilation failed:\n %s", fragment->GetFileInfo().GetRelativePath().string().c_str(), infoLog.data());
			glDeleteShader(fragment->m_id);
			return false;
		}
		PrintLog("Fragment shader %s compiled", fragment->GetFileInfo().GetRelativePath().string().c_str());
		return true;
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateTexture(Resource::Texture* texture)
	{
		const uint32_t wrap = TextureWrappingToAPI(texture->m_wrapping);
		const uint32_t filter = TextureFilteringToAPI(texture->m_filtering);
		const uint32_t format = TextureFormatToAPI(texture->m_format);

		glGenTextures(1, &texture->m_id);
		glBindTexture(GL_TEXTURE_2D, texture->m_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

		glTexImage2D(GL_TEXTURE_2D, 0, format, texture->m_size.x, texture->m_size.y, 0, format, GL_UNSIGNED_BYTE, texture->m_bytes);

		if (texture->m_generateMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::SetTextureWrapping(Resource::Texture* texture, Resource::TextureWrapping wrapping)
	{
		glBindTexture(GL_TEXTURE_2D, texture->m_id);

		auto wrap = TextureWrappingToAPI(wrapping);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

		glBindTexture(GL_TEXTURE_2D, 0);

		texture->m_wrapping = wrapping;
	}

	void Wrapper::RendererAPI::OpenGLRenderer::SetTextureFiltering(Resource::Texture* texture, Resource::TextureFiltering filtering)
	{
		glBindTexture(GL_TEXTURE_2D, texture->m_id);

		auto filter = TextureFilteringToAPI(filtering);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		glBindTexture(GL_TEXTURE_2D, 0);

		texture->m_filtering = filtering;
	}

	void Wrapper::RendererAPI::OpenGLRenderer::DestroyTexture(Resource::Texture* texture)
	{
		if (texture->m_id != 0) 
		{
			glDeleteTextures(1, &texture->m_id);
			texture->m_id = 0;
		}

	}

	uint32_t Wrapper::RendererAPI::OpenGLRenderer::TextureWrappingToAPI(Resource::TextureWrapping filtering)
	{
		switch (filtering)
		{
		case Resource::TextureWrapping::REPEAT:
			return GL_REPEAT;
		case Resource::TextureWrapping::MIRRORED_REPEAT:
			return GL_MIRRORED_REPEAT;
		case Resource::TextureWrapping::CLAMP_TO_EDGE:
			return GL_CLAMP_TO_EDGE;
		case Resource::TextureWrapping::CLAMP_TO_BORDER:
			return GL_CLAMP_TO_BORDER;
		default:
			PrintError("Texture wrapping not recognize");
			return GL_REPEAT;
		}
	}

	uint32_t Wrapper::RendererAPI::OpenGLRenderer::TextureFilteringToAPI(Resource::TextureFiltering filtering)
	{
		switch (filtering)
		{
		case Resource::TextureFiltering::NEAREST:
			return GL_NEAREST;
		case Resource::TextureFiltering::LINEAR:
			return GL_LINEAR;
		default:
			PrintError("Texture filtering not recognize");
			return GL_NEAREST;
		}
	}

	uint32_t Wrapper::RendererAPI::OpenGLRenderer::TextureFormatToAPI(const Resource::TextureFormat format)
	{
		switch (format) {
		case Resource::TextureFormat::RGB:
			return GL_RGB;
		case Resource::TextureFormat::RGBA:
			return GL_RGBA;
		case Resource::TextureFormat::ALPHA:
			return GL_ALPHA;
		case Resource::TextureFormat::LUMINANCE:
			//return GL_LUMINANCE;
		case Resource::TextureFormat::LUMINANCE_ALPHA:
			//return GL_LUMINANCE_ALPHA;
		default:
			PrintError("Texture format not recognize");
			return GL_RGBA;
		}
	}

	int Wrapper::RendererAPI::OpenGLRenderer::GetShaderLocation(const uint32_t id, const std::string& locationName)
	{
		return glGetUniformLocation(id, locationName.c_str());
	}

	Resource::UniformType UniformTypeFromAPI(GLenum type)
	{
		switch (type)
		{
		case GL_FLOAT:
			return Resource::UniformType::Float;
		case GL_FLOAT_VEC2:
			return Resource::UniformType::Float2;
		case GL_FLOAT_VEC3:
			return Resource::UniformType::Float3;
		case GL_FLOAT_VEC4:
			return Resource::UniformType::Float4;
		case GL_INT:
			return Resource::UniformType::Int;
		case GL_INT_VEC2:
			return Resource::UniformType::Int2;
		case GL_INT_VEC3:
			return Resource::UniformType::Int3;
		case GL_INT_VEC4:
			return Resource::UniformType::Int4;
		case GL_SAMPLER_2D:
			return Resource::UniformType::Texture2D;
		case GL_FLOAT_MAT4:
			return Resource::UniformType::Mat4;
		case GL_BOOL:
			return Resource::UniformType::Bool;
		default:
			PrintError("Uniform type not recognize");
			return Resource::UniformType::None;
		}
	}

	UMap<std::string, Resource::Uniform> Wrapper::RendererAPI::OpenGLRenderer::GetShaderUniforms(Resource::Shader* shader)
	{
		if (!shader->IsLoaded() || !shader->HasBeenSent())
			return {};
		UMap<std::string, Resource::Uniform> uniforms;
		auto shaderProgram = shader->p_id;
		GLint numUniforms = 0;
		glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);

		for (GLint i = 0; i < numUniforms; ++i) {
			
			const GLsizei bufSize = 256; // Maximum name length
			GLchar name[bufSize];
			GLsizei length;
			GLint size;
			GLenum type; 
			glGetActiveUniform(shaderProgram, i, bufSize, &length, &size, &type, name);
			
			GLint location = glGetUniformLocation(shaderProgram, name); // Get the location of the uniform
			
			Resource::Uniform uniform = {};
			uniform.location = location;
			uniform.type = UniformTypeFromAPI(type);
			
			std::string uniformName(name);
			if (size_t pos = uniformName.find("material."); pos != std::string::npos)
			{
				uniform.displayName = uniformName.substr(9);
				uniform.shouldDisplay = true;
			}
			
			uniforms[uniformName] = uniform;
		}
		return uniforms;
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendInt(const uint32_t location, const int value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform1i(location, value);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendFloat(const uint32_t location, const float value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform1f(location, value);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendDouble(const uint32_t location, const double value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform1d(location, value);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendVec2f(const uint32_t location, const Vec2f& value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform2fv(location, 1, value.Data());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendVec3f(const uint32_t location, const Vec3f& value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform3fv(location, 1, value.Data());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendVec4f(const uint32_t location, const Vec4f& value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform4fv(location, 1, value.Data());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendVec2i(const uint32_t location, const Vec2i& value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform2iv(location, 1, value.Data());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendVec3i(const uint32_t location, const Vec3i& value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform3iv(location, 1, value.Data());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendVec4i(const uint32_t location, const Vec4i& value)
	{
		if (location == INDEX_NONE)
			return;
		glUniform4iv(location, 1, value.Data());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ShaderSendMat4(const uint32_t location, const Mat4& value, const bool transpose /*= false*/)
	{
		if (location == INDEX_NONE)
			return;
		glUniformMatrix4fv(location, 1, transpose, value.Data());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateVertexArray(uint32_t& vao)
	{
		glGenVertexArrays(1, &vao);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::BindVertexArray(const uint32_t vao)
	{
		glBindVertexArray(vao);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateAndFillVertexBuffer(uint32_t& vbo
		, const std::vector<Vec3f>& positions
		, const std::vector<Vec2f>& textureUVs
		, const std::vector<Vec3f>& normals)
	{
		// Vertex buffer initialization
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Vec3f), positions.data()->Data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		uint32_t m_textureUVs;
		// Texture coordinate buffer initialization
		glGenBuffers(1, &m_textureUVs);
		glBindBuffer(GL_ARRAY_BUFFER, m_textureUVs);
		glBufferData(GL_ARRAY_BUFFER, textureUVs.size() * sizeof(Vec2f), textureUVs.data()->Data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);

		// Normal buffer initialization
		uint32_t m_normals;
		glGenBuffers(1, &m_normals);
		glBindBuffer(GL_ARRAY_BUFFER, m_normals);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Vec3f), normals.data()->Data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateVertexBuffer(uint32_t& vbo, const void* data, const size_t dataSize)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::BindVertexBuffer(const uint32_t vbo)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateIndexBuffer(uint32_t& ebo, const void* data, const size_t dataSize)
	{
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::BindIndexBuffer(const uint32_t ebo)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::VertexAttribPointer(const uint32_t index, const int size, const int stride, const void* pointer)
	{
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
		glEnableVertexAttribArray(index);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::DrawElement(size_t count)
	{
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::UnbindVertexArray()
	{
		glBindVertexArray(0);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::UnbindVertexBuffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::DrawArrays(const size_t start, const size_t count)
	{
		glDrawArrays(GL_TRIANGLES, static_cast<GLsizei>(start), static_cast<GLsizei>(count));

#if WITH_EDITOR
		Editor::UI::EditorUIManager::GetInstance()->GetDebugWindow()->AddTriangleDraw(count / 3);
#endif
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateDynamicVertexBuffer(uint32_t& vao, uint32_t& vbo, size_t dataSize, size_t numVertices)
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, static_cast<int>(numVertices), GL_FLOAT, GL_FALSE, (int)numVertices * sizeof(float), nullptr);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::DrawLine(const Vec3f pos1, const Vec3f pos2, const Vec4f color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
	{
		static float minMaxWidth[2];

		static bool initalized = false;
		static std::weak_ptr<Resource::Shader> unlitShader;
		static uint32_t VAO;
		static uint32_t VBO;
		if (!initalized)
		{
			unlitShader = Resource::ResourceManager::GetInstance()->GetUnlitShader();
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) + 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), &pos1);
			glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float), 3 * sizeof(float), &pos2);

			// position attribute
			glVertexAttribPointer(0U, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(0));
			glEnableVertexAttribArray(0U);
			initalized = true;

			// Query the maximum supported line width range
			glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, minMaxWidth);
		}
		if (!unlitShader.lock() || !unlitShader.lock()->HasBeenSent())
		{
			unlitShader = Resource::ResourceManager::GetInstance()->GetUnlitShader();
			return;
		}

		if (lineWidth < minMaxWidth[0] || lineWidth > minMaxWidth[1])
			lineWidth = 1.f;

		// Bind Position
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), &pos1);
		glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float), 3 * sizeof(float), &pos2);

		//glDepthRange(0, 0.01);
		float defaultWidth;
		glGetFloatv(GL_LINE_WIDTH, &defaultWidth);
		glLineWidth(lineWidth);
		const Shared<Resource::Shader> shader = unlitShader.lock();

		shader->Use();

		const auto& VP = Core::SceneHolder::GetCurrentScene()->GetVP();

		shader->SendMat4("MVP", VP);
		shader->SendVec4f("material.diffuse", color);
		shader->SendInt("material.hasAlbedo", false);

		// Draw vertices
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);

		glLineWidth(defaultWidth);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::BindTexture(Resource::Texture* texture, const uint32_t id /*= 0*/)
	{
		ASSERT(texture->HasBeenSent());
		glActiveTexture(GL_TEXTURE0 + id);
		glBindTexture(GL_TEXTURE_2D, texture->GetID());
	}

	void Wrapper::RendererAPI::OpenGLRenderer::UnbindTexture()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateCubemap(Resource::Cubemap* cubemap)
	{
		glGenTextures(1, &cubemap->m_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->m_id);
		
	}

	void Wrapper::RendererAPI::OpenGLRenderer::SetCubemapFace(int face, const Wrapper::Image& image)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, image.size.x, image.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::SetCubemapParameters()
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::DestroyCubemap(Resource::Cubemap* cubemap)
	{
		glDeleteTextures(1, &cubemap->m_id);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::BindCubemap(Resource::Cubemap* cubemap, uint32_t id)
	{
		ASSERT(cubemap->HasBeenSent());
		glActiveTexture(GL_TEXTURE0 + id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->m_id);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::CreateRenderBuffer(Render::Framebuffer* framebuffer)
	{
		if (!framebuffer)
			return;
		framebuffer->m_renderTexture->m_bytes = nullptr;
		framebuffer->m_renderTexture->p_shouldBeLoaded = true;
		framebuffer->m_renderTexture->p_loaded = true;
		framebuffer->m_renderTexture->m_format = Resource::TextureFormat::RGBA;
		framebuffer->m_renderTexture->m_size = framebuffer->m_size;

		CreateTexture(framebuffer->m_renderTexture.get());
		framebuffer->m_renderTexture->p_hasBeenSent = true;

		framebuffer->m_renderTexture->Bind();

		glGenFramebuffers(1, &framebuffer->m_frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->m_frameBuffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->m_renderTexture->m_id, 0);
		// Set texture to resourcesManager.
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
			PrintLog("Framebuffer %d Complete !", framebuffer->m_frameBuffer);
		}
		else
			PrintError("Framebuffer %d Not Complete !", framebuffer->m_frameBuffer);

		glGenRenderbuffers(1, &framebuffer->m_renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->m_renderBuffer);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)framebuffer->m_size.x, (GLsizei)framebuffer->m_size.y); // use a single render buffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->m_renderBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::BindRenderBuffer(Render::Framebuffer* framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->m_frameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->m_renderBuffer);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::UnbindRenderBuffer(Render::Framebuffer* framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::DeleteRenderBuffer(Render::Framebuffer* framebuffer)
	{
		glDeleteFramebuffers(1, &framebuffer->m_frameBuffer);
		glDeleteRenderbuffers(1, &framebuffer->m_renderBuffer);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ResizeRenderBuffer(Render::Framebuffer* framebuffer, const Vec2i& size)
	{
		if (size.x * size.y != 0) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, GL_RGBA, GL_FLOAT, nullptr);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
		}
	}

	void Wrapper::RendererAPI::OpenGLRenderer::SetViewport(const Vec2i& size)
	{
		glViewport(0, 0, size.x, size.y);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::EnableDepth(const bool active /*= true*/)
	{
		if (active)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}


	void Wrapper::RendererAPI::OpenGLRenderer::SetDepthRange(const float _near, const float _far)
	{
		glDepthRange(static_cast<double>(_near), static_cast<double>(_far));
	}

	Vec4f Wrapper::RendererAPI::OpenGLRenderer::ReadPixelColor(const Vec2f& mousePos)
	{
		unsigned char data[4];
		glFlush();
		glFinish();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glReadPixels(static_cast<GLint>(mousePos.x), static_cast<GLint>(mousePos.y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		return Vec4f(data[0], data[1], data[2], data[3]);
	}

	void Wrapper::RendererAPI::OpenGLRenderer::ReadPixels(const Vec2i& size, unsigned char*& data)
	{
		glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

}
