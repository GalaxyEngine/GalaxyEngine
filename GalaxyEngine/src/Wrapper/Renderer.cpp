#include "pch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Wrapper/Renderer.h"
#include "Wrapper/Window.h"

#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

#include "Render/Framebuffer.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"

namespace GALAXY {
	std::unique_ptr<Wrapper::Renderer> Wrapper::Renderer::m_instance = nullptr;

	void Wrapper::Renderer::CreateInstance(RenderAPI renderAPI)
	{
		switch (renderAPI)
		{
		case RenderAPI::OPENGL:
		{
			m_instance = std::make_unique<Wrapper::OpenGLRenderer>();
			break;
		}
		default:
			break;
		}
		m_instance->Initialize();
		m_instance->EnableDebugOutput();
	}

   void Wrapper::Renderer::DrawWireCube(const Vec3f &pos, const Vec3f &size, const Vec4f& color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
   {    
		// Define the eight vertices of the cube
		Vec3f vertices[8];
		vertices[0] = pos + Vec3f(-size.x, -size.y, -size.z);
		vertices[1] = pos + Vec3f(size.x, -size.y, -size.z);
		vertices[2] = pos + Vec3f(size.x, size.y, -size.z);
		vertices[3] = pos + Vec3f(-size.x, size.y, -size.z);
		vertices[4] = pos + Vec3f(-size.x, -size.y, size.z);
		vertices[5] = pos + Vec3f(size.x, -size.y, size.z);
		vertices[6] = pos + Vec3f(size.x, size.y, size.z);
		vertices[7] = pos + Vec3f(-size.x, size.y, size.z);

		// Draw the edges of the cube
		DrawLine(vertices[0], vertices[1], color, lineWidth);
		DrawLine(vertices[1], vertices[2], color, lineWidth);
		DrawLine(vertices[2], vertices[3], color, lineWidth);
		DrawLine(vertices[3], vertices[0], color, lineWidth);
		DrawLine(vertices[4], vertices[5], color, lineWidth);
		DrawLine(vertices[5], vertices[6], color, lineWidth);
		DrawLine(vertices[6], vertices[7], color, lineWidth);
		DrawLine(vertices[7], vertices[4], color, lineWidth);
		DrawLine(vertices[0], vertices[4], color, lineWidth);
		DrawLine(vertices[1], vertices[5], color, lineWidth);
		DrawLine(vertices[2], vertices[6], color, lineWidth);
		DrawLine(vertices[3], vertices[7], color, lineWidth);
   }
	
   void Wrapper::Renderer::DrawWireCubeMinMax(const Vec3f& min, const Vec3f& max, const Vec4f& color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
   {
	   // Define the eight vertices of the cube
	   Vec3f vertices[8];
	   vertices[0] = min;
	   vertices[1] = Vec3f(max.x, min.y, min.z);
	   vertices[2] = Vec3f(max.x, max.y, min.z);
	   vertices[3] = Vec3f(min.x, max.y, min.z);
	   vertices[4] = Vec3f(min.x, min.y, max.z);
	   vertices[5] = Vec3f(max.x, min.y, max.z);
	   vertices[6] = max;
	   vertices[7] = Vec3f(min.x, max.y, max.z);

	   // Draw the edges of the cube
	   DrawLine(vertices[0], vertices[1], color, lineWidth);
	   DrawLine(vertices[1], vertices[2], color, lineWidth);
	   DrawLine(vertices[2], vertices[3], color, lineWidth);
	   DrawLine(vertices[3], vertices[0], color, lineWidth);
	   DrawLine(vertices[4], vertices[5], color, lineWidth);
	   DrawLine(vertices[5], vertices[6], color, lineWidth);
	   DrawLine(vertices[6], vertices[7], color, lineWidth);
	   DrawLine(vertices[7], vertices[4], color, lineWidth);
	   DrawLine(vertices[0], vertices[4], color, lineWidth);
	   DrawLine(vertices[1], vertices[5], color, lineWidth);
	   DrawLine(vertices[2], vertices[6], color, lineWidth);
	   DrawLine(vertices[3], vertices[7], color, lineWidth);
   }

   void Wrapper::Renderer::DrawWireCube(const Vec3f& pos, const Vec3f& size, const Quat& rotation, const Vec4f& color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
   {
	   // Define the eight vertices of the cube
	   Vec3f vertices[8];
	   vertices[0] = pos + rotation * Vec3f(-size.x, -size.y, -size.z);
	   vertices[1] = pos + rotation * Vec3f(size.x, -size.y, -size.z);
	   vertices[2] = pos + rotation * Vec3f(size.x, size.y, -size.z);
	   vertices[3] = pos + rotation * Vec3f(-size.x, size.y, -size.z);
	   vertices[4] = pos + rotation * Vec3f(-size.x, -size.y, size.z);
	   vertices[5] = pos + rotation * Vec3f(size.x, -size.y, size.z);
	   vertices[6] = pos + rotation * Vec3f(size.x, size.y, size.z);
	   vertices[7] = pos + rotation * Vec3f(-size.x, size.y, size.z);

	   // Draw the edges of the cube
	   DrawLine(vertices[0], vertices[1], color, lineWidth);
	   DrawLine(vertices[1], vertices[2], color, lineWidth);
	   DrawLine(vertices[2], vertices[3], color, lineWidth);
	   DrawLine(vertices[3], vertices[0], color, lineWidth);
	   DrawLine(vertices[4], vertices[5], color, lineWidth);
	   DrawLine(vertices[5], vertices[6], color, lineWidth);
	   DrawLine(vertices[6], vertices[7], color, lineWidth);
	   DrawLine(vertices[7], vertices[4], color, lineWidth);
	   DrawLine(vertices[0], vertices[4], color, lineWidth);
	   DrawLine(vertices[1], vertices[5], color, lineWidth);
	   DrawLine(vertices[2], vertices[6], color, lineWidth);
	   DrawLine(vertices[3], vertices[7], color, lineWidth);
   }

   void Wrapper::Renderer::DrawWireCircle(const Vec3f& pos, const Vec3f& normal, float radius, int numSegments /*= 32*/, Vec4f color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
   {
	   Vec3f temp = (normal.x < normal.z) ? Vec3f::Right() : Vec3f::Forward();
	   Vec3f forward = normal.Cross(temp).GetNormalize();
	   Vec3f right = forward.Cross(normal).GetNormalize();

	   Vec3f prevPt = pos + (forward * radius);
	   float angleStep = (PI * 2.f) / numSegments;
	   for (int i = 0; i < numSegments; i++)
	   {
		   float angle = (i == numSegments - 1) ? 0.f : (i + 1) * angleStep;

		   Vec3f nextPtLocal = Vec3f(std::sin(angle), 0.f, std::cos(angle)) * radius;

		   Vec3f nextPt = pos + (right * nextPtLocal.x) + (forward * nextPtLocal.z);

		   DrawLine(prevPt, nextPt, color, lineWidth);

		   prevPt = nextPt;
	   }
   }

   // OpenGL Renderer
   Wrapper::OpenGLRenderer::OpenGLRenderer() {}

	Wrapper::OpenGLRenderer::~OpenGLRenderer() {}

	void Wrapper::OpenGLRenderer::Initialize()
	{
		if (!gladLoadGLLoader((GLADloadproc)(Window::GetProcAddress))) {
			PrintError("Failed to initialize GLAD");
			return;
		}

		const GLubyte* glVersion = glGetString(GL_VERSION);
		PrintLog("Loaded OpenGL %s\n", glVersion);
		p_initalized = true;
	}

	void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131154) return;
		if (id == 7) return;// lineWidth

		std::string log = "";
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

	void Wrapper::OpenGLRenderer::EnableDebugOutput()
	{
		GLint flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		//if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(DebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		//}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void Wrapper::OpenGLRenderer::UseShader(Resource::Shader* shader)
	{
		glUseProgram(shader->p_id);
	}

	void Wrapper::OpenGLRenderer::Viewport(const Vec2i& pos, const Vec2i& size)
	{
		glViewport(pos.x, pos.y, size.x, size.y);
	}

	void Wrapper::OpenGLRenderer::ClearColorAndBuffer(const Vec4f& color)
	{
		glClearColor(color.x, color.y, color.z, color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	bool Wrapper::OpenGLRenderer::LinkShaders(Resource::Shader* shader)
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
			glGetProgramInfoLog(shader->p_id, 512, NULL, infoLog);
			PrintError("Error when Link shader %s :\n %s", shader->GetFileInfo().GetRelativePath().string().c_str(), infoLog);
			return false;
		}
		PrintLog("Linked shader %s", shader->GetFileInfo().GetRelativePath().string().c_str());
		return true;
	}

	bool Wrapper::OpenGLRenderer::CompileVertexShader(Resource::VertexShader* vertex)
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

	bool Wrapper::OpenGLRenderer::CompileFragmentShader(Resource::FragmentShader* fragment)
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

	void Wrapper::OpenGLRenderer::CreateTexture(Resource::Texture* texture)
	{
		uint32_t filter = TextureFilteringToAPI(texture->m_filtering);
		uint32_t format = TextureFormatToAPI(texture->m_format);

		glGenTextures(1, &texture->m_id);
		glBindTexture(GL_TEXTURE_2D, texture->m_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, filter);

		glTexImage2D(GL_TEXTURE_2D, 0, format, texture->m_size.x, texture->m_size.y, 0, format, GL_UNSIGNED_BYTE, texture->m_bytes);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Wrapper::OpenGLRenderer::DestroyTexture(Resource::Texture* texture)
	{
		if (texture->HasBeenSent())
			glDeleteTextures(1, &texture->m_id);
	}

	uint32_t Wrapper::OpenGLRenderer::TextureFilteringToAPI(Resource::TextureFiltering filtering)
	{
		switch (filtering)
		{
		case Resource::TextureFiltering::REPEAT:
			return GL_REPEAT;
		case Resource::TextureFiltering::MIRRORED_REPEAT:
			return GL_MIRRORED_REPEAT;
		case Resource::TextureFiltering::CLAMP_TO_EDGE:
			return GL_CLAMP_TO_EDGE;
		case Resource::TextureFiltering::CLAMP_TO_BORDER:
			return GL_CLAMP_TO_BORDER;
		default:
			PrintError("Texture filtering not recognize");
			return GL_REPEAT;
		}
	}

	uint32_t Wrapper::OpenGLRenderer::TextureFormatToAPI(Resource::TextureFormat format)
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
			//return GL_LUMINANCE_ALPHA
		default:
			PrintError("Texture format not recognize");
			return GL_RGBA;
		}
	}

	int Wrapper::OpenGLRenderer::GetShaderLocation(uint32_t id, const std::string& locationName)
	{
		return glGetUniformLocation(id, locationName.c_str());
	}

	void Wrapper::OpenGLRenderer::ShaderSendInt(uint32_t location, int value)
	{
		if (location == -1)
			return;
		glUniform1i(location, value);
	}

	void Wrapper::OpenGLRenderer::ShaderSendFloat(uint32_t location, float value)
	{
		if (location == -1)
			return;
		glUniform1f(location, value);
	}

	void Wrapper::OpenGLRenderer::ShaderSendDouble(uint32_t location, double value)
	{
		if (location == -1)
			return;
		glUniform1d(location, value);
	}

	void Wrapper::OpenGLRenderer::ShaderSendVec2f(uint32_t location, const Vec2f& value)
	{
		if (location == -1)
			return;
		glUniform2fv(location, 1, value.Data());
	}

	void Wrapper::OpenGLRenderer::ShaderSendVec3f(uint32_t location, const Vec3f& value)
	{
		if (location == -1)
			return;
		glUniform3fv(location, 1, value.Data());
	}

	void Wrapper::OpenGLRenderer::ShaderSendVec4f(uint32_t location, const Vec4f& value)
	{
		if (location == -1)
			return;
		glUniform4fv(location, 1, value.Data());
	}

	void Wrapper::OpenGLRenderer::ShaderSendVec2i(uint32_t location, const Vec2i& value)
	{
		if (location == -1)
			return;
		glUniform2iv(location, 1, value.Data());
	}

	void Wrapper::OpenGLRenderer::ShaderSendVec3i(uint32_t location, const Vec3i& value)
	{
		if (location == -1)
			return;
		glUniform3iv(location, 1, value.Data());
	}

	void Wrapper::OpenGLRenderer::ShaderSendVec4i(uint32_t location, const Vec4i& value)
	{
		if (location == -1)
			return;
		glUniform4iv(location, 1, value.Data());
	}

	void Wrapper::OpenGLRenderer::ShaderSendMat4(uint32_t location, const Mat4& value, bool transpose)
	{
		if (location == -1)
			return;
		glUniformMatrix4fv(location, 1, transpose, value.Data());
	}

	void Wrapper::OpenGLRenderer::CreateVertexArray(uint32_t& vao)
	{
		glGenVertexArrays(1, &vao);
	}

	void Wrapper::OpenGLRenderer::BindVertexArray(uint32_t vao)
	{
		glBindVertexArray(vao);
	}

	void Wrapper::OpenGLRenderer::CreateAndFillVertexBuffer(uint32_t& vbo
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

	void Wrapper::OpenGLRenderer::CreateVertexBuffer(uint32_t& vbo, const void* data, size_t dataSize)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	}

	void Wrapper::OpenGLRenderer::BindVertexBuffer(uint32_t vbo)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}

	void Wrapper::OpenGLRenderer::CreateIndexBuffer(uint32_t& ebo, const void* data, size_t dataSize)
	{
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	}

	void Wrapper::OpenGLRenderer::BindIndexBuffer(uint32_t ebo)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	}

	void Wrapper::OpenGLRenderer::VertexAttribPointer(uint32_t index, int size, int stride, const void* pointer)
	{
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
		glEnableVertexAttribArray(index);
	}

	void Wrapper::OpenGLRenderer::DrawElement(size_t count)
	{
		glDrawElements(GL_TRIANGLES, (GLsizei)count, GL_UNSIGNED_INT, nullptr);
	}

	void Wrapper::OpenGLRenderer::UnbindVertexArray()
	{
		glBindVertexArray(0);
	}

	void Wrapper::OpenGLRenderer::UnbindVertexBuffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Wrapper::OpenGLRenderer::DrawArrays(size_t start, size_t count)
	{
		glDrawArrays(GL_TRIANGLES, (GLsizei)start, (GLsizei)count);
	}

	void Wrapper::OpenGLRenderer::DrawLine(Vec3f pos1, Vec3f pos2, Vec4f color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
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
			glVertexAttribPointer(0U, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
		auto shader = unlitShader.lock();

		shader->Use();

		const auto& VP = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetVP();

		ShaderSendMat4(shader->GetLocation("MVP"), VP, true);
		ShaderSendVec4f(shader->GetLocation("Diffuse"), color);
		ShaderSendInt(shader->GetLocation("EnableTexture"), false);

		// Draw vertices
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);

		glLineWidth(defaultWidth);
	}

	void Wrapper::OpenGLRenderer::BindTexture(Resource::Texture* texture, uint32_t id /*= 0*/)
	{
		glActiveTexture(GL_TEXTURE0 + id);
		glBindTexture(GL_TEXTURE_2D, texture->GetID());
	}

	void Wrapper::OpenGLRenderer::UnbindTexture()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Wrapper::OpenGLRenderer::CreateRenderBuffer(Render::Framebuffer* framebuffer)
	{
		if (!framebuffer)
			return;
		framebuffer->m_renderTexture->m_bytes = nullptr;
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

	void Wrapper::OpenGLRenderer::BindRenderBuffer(Render::Framebuffer* framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->m_frameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->m_renderBuffer);
	}

	void Wrapper::OpenGLRenderer::UnbindRenderBuffer(Render::Framebuffer* framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void Wrapper::OpenGLRenderer::DeleteRenderBuffer(Render::Framebuffer* framebuffer)
	{
		glDeleteFramebuffers(1, &framebuffer->m_frameBuffer);
		glDeleteRenderbuffers(1, &framebuffer->m_renderBuffer);
	}

	void Wrapper::OpenGLRenderer::ResizeRenderBuffer(Render::Framebuffer* framebuffer, const Vec2i& size)
	{
		if (size.x * size.y != 0) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGBA, GL_FLOAT, NULL);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)size.x, (GLsizei)size.y);
		}
	}

	void Wrapper::OpenGLRenderer::ActiveDepth(bool active /*= true*/)
	{
		if (active)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	
	void Wrapper::OpenGLRenderer::SetDepthRange(float _near, float _far)
	{
		glDepthRange((double)_near, (double)_far);
	}

	Vec4f Wrapper::OpenGLRenderer::ReadPixelColor(const Vec2f& mousePos)
	{
		unsigned char data[4];
		glFlush();
		glFinish();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glReadPixels((GLint)mousePos.x, (GLint)mousePos.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		return Vec4f(data[0], data[1], data[2], data[3]);
	}

}