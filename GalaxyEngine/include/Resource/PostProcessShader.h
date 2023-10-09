#pragma once
#include "GalaxyAPI.h"
#include "Shader.h"
namespace GALAXY 
{
	namespace Resource
	{
		class PostProcessShader : public Shader
		{
		public:
			PostProcessShader(const std::filesystem::path& fullPath);

			void Load() override;

			static ResourceType GetResourceType() { return ResourceType::PostProcessShader; }
		};
	}
}
