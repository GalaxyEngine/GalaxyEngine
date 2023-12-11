#include "pch.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/ResourceManager.h"

#include "Wrapper/OBJLoader.h"

namespace GALAXY {
	Resource::Model::~Model()
	{
		m_meshes.clear();
	}

	void Resource::Model::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		if (p_fileInfo.GetExtension() == ".fbx")
		{
			m_modelType = Resource::ModelExtension::FBX;
		}
		else if (p_fileInfo.GetExtension() == ".obj")
		{
			m_modelType = Resource::ModelExtension::OBJ;
			Wrapper::OBJLoader::Load(p_fileInfo.GetFullPath(), this);
		}

		CreateDataFile();
		SendRequest();
	}

	void Resource::Model::Serialize(Utils::Serializer& serializer) const
	{
		IResource::Serialize(serializer);
		serializer << Pair::BEGIN_MAP << "Model";
		serializer << Pair::KEY << "Mesh Count" << Pair::VALUE << m_meshes.size();
		serializer << Pair::BEGIN_TAB;
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			serializer << Pair::KEY << "Mesh " + std::to_string(i) << Pair::VALUE << m_meshes[i].lock()->GetFileInfo().GetFileName();
		}
		serializer << Pair::END_TAB;
		serializer << Pair::END_MAP << "Model";
	}

	void Resource::Model::Deserialize(Utils::Parser& parser)
	{
		IResource::Deserialize(parser);
		parser.NewDepth();
		const size_t meshCount = parser["Mesh Count"].As<size_t>();
		for (int i = 0; i < meshCount; i++)
		{
			std::string meshName = parser["Mesh " + std::to_string(i)];
			auto meshPath = Mesh::CreateMeshPath(GetFileInfo().GetFullPath(), meshName);
			Resource::ResourceManager::AddResource<Mesh>(meshPath);
		}
	}

	void Resource::Model::ComputeBoundingBox()
	{
		for (auto& weakMesh : m_meshes)
		{
			const Shared<Mesh> mesh = weakMesh.lock();
			mesh->ComputeBoundingBox();

			m_boundingBox.min.x = std::min(m_boundingBox.min.x, mesh->m_boundingBox.min.x);
			m_boundingBox.min.y = std::min(m_boundingBox.min.y, mesh->m_boundingBox.min.y);
			m_boundingBox.min.z = std::min(m_boundingBox.min.z, mesh->m_boundingBox.min.z);

			m_boundingBox.max.x = std::max(m_boundingBox.max.x, mesh->m_boundingBox.max.x);
			m_boundingBox.max.y = std::max(m_boundingBox.max.y, mesh->m_boundingBox.max.y);
			m_boundingBox.max.z = std::max(m_boundingBox.max.z, mesh->m_boundingBox.max.z);
		}

		m_boundingBox.center = (m_boundingBox.min + m_boundingBox.max) / 2.0f;
	}

}