#include "pch.h"
#include "Resource/Model.h"
#include "Wrapper/OBJLoader.h"

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
}
