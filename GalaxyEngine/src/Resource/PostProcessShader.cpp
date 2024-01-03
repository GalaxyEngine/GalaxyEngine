#include "pch.h"
#include "Resource/PostProcessShader.h"
#include "Resource/ResourceManager.h"
#define VERTEX_PP_PATH "CoreResources/shaders/PostProcess/postprocess.vert"

namespace GALAXY 
{

	Resource::PostProcessShader::PostProcessShader(const Path& fullPath) : Shader(fullPath)
	{
	}

	void Resource::PostProcessShader::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;

		Weak<PostProcessShader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::PostProcessShader>(p_fileInfo.GetFullPath());
		auto vertexShader = ResourceManager::GetOrLoad<VertexShader>(VERTEX_PP_PATH);
		SetVertex(vertexShader.lock(), thisShader, false);

		auto renderer = Wrapper::Renderer::GetInstance();
		if (std::fstream file = Utils::FileSystem::OpenFile(p_fileInfo.GetFullPath()); file.is_open())
		{
			Weak<Shader> this_shader = ResourceManager::GetInstance()->GetResource<Resource::Shader>(p_fileInfo.GetFullPath());

			// Parse .shader file
			std::string line;
			while (std::getline(file, line)) {
				if (line[0] == 'V')
				{
					Path vertPath = line.substr(4);
					vertPath = p_fileInfo.GetFullPath().parent_path() / vertPath;
					Weak<VertexShader> vertexShader = ResourceManager::GetOrLoad<Resource::VertexShader>(vertPath);
					SetVertex(vertexShader.lock(), this_shader);
				}
				else if (line[0] == 'G')
				{
					Path geomPath = line.substr(4);
					geomPath = p_fileInfo.GetFullPath().parent_path() / geomPath;
					Weak<GeometryShader> geometryShader = ResourceManager::GetOrLoad<Resource::GeometryShader>(geomPath);
					SetGeometry(geometryShader.lock(), this_shader);
				}
				else if (line[0] == 'F')
				{
					Path fragPath = line.substr(4);
					fragPath = p_fileInfo.GetFullPath().parent_path() / fragPath;
					Weak<FragmentShader> fragmentShader = ResourceManager::GetOrLoad<Resource::FragmentShader>(fragPath);
					SetFragment(fragmentShader.lock(), this_shader);
				}
			}
			SendRequest();
		}
	}

}
