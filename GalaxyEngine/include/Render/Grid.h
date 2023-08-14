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
			virtual ~Grid() {}

			std::weak_ptr<Resource::Shader> m_shader;

			void Initialize();
			
			void Draw();

		private:
			uint32_t vao, vbo;

		};
	}
}
