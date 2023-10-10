#include "pch.h"
#include "Resource/PostProcessShader.h"
#include "Resource/ResourceManager.h"
#define VERTEX_PP_PATH "CoreResources\\shaders\\PostProcess\\postprocess.vert"

namespace GALAXY 
{

	Resource::PostProcessShader::PostProcessShader(const std::filesystem::path& fullPath) : Shader(fullPath)
	{
		p_renderer = Wrapper::Renderer::GetInstance();
	}

	void Resource::PostProcessShader::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;

		std::weak_ptr<PostProcessShader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::PostProcessShader>(p_fileInfo.GetFullPath());
		auto vertexShader = ResourceManager::GetInstance()->GetOrLoad<VertexShader>(VERTEX_PP_PATH);
		SetVertex(vertexShader, thisShader);

		p_renderer = Wrapper::Renderer::GetInstance();
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

}
