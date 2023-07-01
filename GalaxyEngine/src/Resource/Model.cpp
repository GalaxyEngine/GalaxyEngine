#include "pch.h"
#include "Resource/Model.h"
#include "Wrapper/OBJLoader.h"

void Resource::Model::Load()
{
	auto extension = GetRelativePath().substr(GetRelativePath().find_last_of('.'));
	if (extension == ".fbx")
	{
		m_modelType = Resource::ModelExtension::FBX;
	}
	else if (extension == ".obj")
	{
		m_modelType = Resource::ModelExtension::OBJ;
		Wrapper::OBJLoader::Load(p_fullPath, this);
	}
}
