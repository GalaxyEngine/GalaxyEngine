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
			PostProcessShader(const Path& fullPath);

			void Load() override;

			static inline ResourceType GetResourceType() { return ResourceType::PostProcessShader; }
		};
	}
}
