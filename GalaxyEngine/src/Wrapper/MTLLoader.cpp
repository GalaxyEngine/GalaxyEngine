#include "pch.h"
#include "Wrapper/MTLLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Material.h"

namespace GALAXY 
{

	bool Wrapper::MTLLoader::Load(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		if (!file.is_open()) {
			PrintError("Failed to open MTL file %s", path.c_str());
			return false;
		}

		auto endMaterial = [&](std::shared_ptr<Resource::Material> material)
			{
				if (material && !material->p_fileInfo.Exist())
				{
					material->Save();
				}
			};

		std::string line;
		std::shared_ptr<Resource::Material> currentMaterial;
		while (std::getline(file, line)) {
			std::istringstream iss(line);
			std::string token;
			iss >> token;
			// Ambient
			if (token == "newmtl")
			{
				endMaterial(currentMaterial);
				std::string name;
				iss >> name;
				const std::filesystem::path& matFullPath = path.parent_path() / name.append(".mat");
				if (currentMaterial = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Material>(matFullPath).lock())
				{
					// file .mat already exists, so skip this one to go to the next material
					while (std::getline(file, line))
						if (line.empty() || line == "\n" || line == "\0")
							break;
					continue;
				}
				else
				{
					currentMaterial = std::make_shared<Resource::Material>(matFullPath);
					Resource::ResourceManager::GetInstance()->AddResource(currentMaterial);
				}
			}
			if (token == "Ka")
			{
				Vec3f ambient;
				iss >> ambient.x >> ambient.y >> ambient.z;
				currentMaterial->m_ambient = ambient;
			}
			// Diffuse
			else if (token == "Kd")
			{
				Vec3f diffuse;
				iss >> diffuse.x >> diffuse.y >> diffuse.z;
				currentMaterial->m_diffuse = diffuse;
			}
			// Specular
			else if (token == "Ks")
			{
				Vec3f specular;
				iss >> specular.x >> specular.y >> specular.z;
				currentMaterial->m_specular = specular;
			}
			// Emissive
			else if (token == "d")
			{
				float transparency;
				iss >> transparency;
				currentMaterial->m_ambient.w = transparency;
				currentMaterial->m_diffuse.w = transparency;
				currentMaterial->m_specular.w = transparency;
			}
			else if (token == "map_Kd")
			{
				std::filesystem::path texPath;
				iss >> texPath;
				texPath = path.parent_path() / texPath;
				currentMaterial->m_albedo = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(texPath);
			}
		}
		endMaterial(currentMaterial);
	}

}
