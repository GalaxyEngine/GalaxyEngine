#pragma once
#include "GalaxyAPI.h"

#include <filesystem>

namespace ofbx
{
	struct IScene;
	struct Material;
}

namespace GALAXY
{
	namespace Resource { class Model; class Material; }
	namespace Wrapper {
		class FBXLoader
		{
		public:
			static void Load(const std::filesystem::path& fullPath, Resource::Model* outputModel);
		private:
			static void LoadTextures(ofbx::IScene* fbxScene, const std::filesystem::path& fullPath);
			static void LoadModel(ofbx::IScene* fbxScene, const std::filesystem::path& fullPath, Resource::Model* outputModel);

		};
	}
}
