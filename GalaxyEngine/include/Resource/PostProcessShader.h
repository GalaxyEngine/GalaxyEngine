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
			explicit PostProcessShader(const Path& fullPath);
			PostProcessShader& operator=(const PostProcessShader& other) = default;
			PostProcessShader(const PostProcessShader&) = default;
			PostProcessShader(PostProcessShader&&) noexcept = default;
			~PostProcessShader() override = default;

			void Load() override;

			const char* GetResourceName() const override { return "Post Process Shader"; }

			static inline ResourceType GetResourceType() { return ResourceType::PostProcessShader; }
		};
	}
}
