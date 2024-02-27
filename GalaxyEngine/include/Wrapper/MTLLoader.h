#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Wrapper
	{
		class MTLLoader
		{
		public:
			MTLLoader() {}

			static bool Load(const std::filesystem::path& path);

			// Return the path of the material with the mtl path and the name of the material
			static std::filesystem::path GetMaterialPath(const std::filesystem::path& mtlPath, const std::string& materialName);
		private:

		};
	}
}
