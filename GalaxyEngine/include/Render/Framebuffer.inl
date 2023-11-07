#include "Framebuffer.h"
namespace GALAXY 
{
	Weak<Resource::PostProcessShader> Render::Framebuffer::GetPostProcessShader()
	{
		if (!m_postProcess)
			return Weak<Resource::PostProcessShader>();
		return m_postProcess->m_shader;
	}
}
