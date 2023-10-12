#include "pch.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"

#include "Wrapper/OBJLoader.h"

namespace GALAXY {
	Resource::Model::~Model()
	{
		m_meshes.clear();
	}

	void Resource::Model::Load()
	{
		if (p_fileInfo.GetExtension() == ".fbx")
		{
			m_modelType = Resource::ModelExtension::FBX;
		}
		else if (p_fileInfo.GetExtension() == ".obj")
		{
			m_modelType = Resource::ModelExtension::OBJ;
			Wrapper::OBJLoader::Load(p_fileInfo.GetFullPath(), this);
		}

		CreateDataFiles();
	}

	void Resource::Model::CreateDataFiles()
	{
		auto fileWithoutExt = p_fileInfo.GetFullPath().parent_path() / p_fileInfo.GetFileNameNoExtension();
		std::filesystem::path path = fileWithoutExt.wstring() + L".gdata";
		if (auto& file = Utils::FileSystem::GenerateFile(path); file.is_open()) {
			file << "Origin: " << p_fileInfo.GetRelativePath().string() << std::endl;
			for (auto& mesh : m_meshes)
			{
				file << '\t' << mesh.lock()->GetFileInfo().GetFileName() << std::endl;
			}
			file.close();
		}
	}
}