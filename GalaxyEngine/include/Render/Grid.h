#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Resource
	{
		class Shader;
	}
	namespace Render
	{
		class Grid
		{
		public:
			Grid() {}
			Grid& operator=(const Grid& other) = default;
			Grid(const Grid&) = default;
			Grid(Grid&&) noexcept = default;
			virtual ~Grid();

			void Initialize();
			
			void Draw() const;

		private:

			Weak<Resource::Shader> m_shader;

			uint32_t m_vao = -1, m_vbo = -1;

		};
	}
}
