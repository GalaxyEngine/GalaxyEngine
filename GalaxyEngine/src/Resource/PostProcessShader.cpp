#include "pch.h"
#include "Resource/PostProcessShader.h"
#include "Resource/ResourceManager.h"
#define VERTEX_PP_PATH "CoreResources\\shaders\\PostProcess\\postprocess.vert"

namespace GALAXY 
{

	Resource::PostProcessShader::PostProcessShader(const std::filesystem::path& fullPath) : Shader(fullPath)
	{
	}

	void Resource::PostProcessShader::Load()
	{
		Shader::Load();
		if (p_shouldBeLoaded)
			return;
		std::weak_ptr<PostProcessShader> thisShader = ResourceManager::GetInstance()->GetResource<Resource::PostProcessShader>(p_fileInfo.GetFullPath());
		SetVertex(ResourceManager::GetInstance()->GetOrLoad<VertexShader>(VERTEX_PP_PATH), thisShader);
	}

}
