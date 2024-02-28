#include "pch.h"

#include "Resource/Model.h"

#include "Component/MeshComponent.h"
#include "Core/Application.h"
#include "Editor/ThumbnailCreator.h"
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
		if (Editor::ThumbnailCreator::IsThumbnailUpToDate(this))
			return;
		CreateThumbnail();
	}

	Shared<Core::GameObject> Resource::Model::ToGameObject()
	{
		Shared<Core::GameObject> root = std::make_shared<Core::GameObject>(GetFileInfo().GetFileNameNoExtension());
		size_t materialIndex = 0;
		for (auto& mesh : m_meshes)
		{
			Shared<Core::GameObject> meshGO = std::make_shared<Core::GameObject>(mesh.lock()->GetFileInfo().GetFileNameNoExtension());
			auto meshComponent = meshGO->AddComponent<Component::MeshComponent>();
			meshComponent.lock()->SetMesh(mesh);
			for (auto& subMesh : mesh.lock()->m_subMeshes) {
				if (materialIndex < m_materials.size())
					meshComponent.lock()->AddMaterial(m_materials[materialIndex++]);
				else
					meshComponent.lock()->AddMaterial(ResourceManager::GetInstance()->GetDefaultMaterial());
			}
			if (m_meshes.size() != 1)
				root->AddChild(meshGO);
			else
				root = std::move(meshGO); // Only one mesh, return it
		}
		return root;
	}

	void Resource::Model::Serialize(CppSer::Serializer& serializer) const
	{
		IResource::Serialize(serializer);
		serializer << CppSer::Pair::BeginMap << "Model";
		serializer << CppSer::Pair::Key << "Mesh Count" << CppSer::Pair::Value << m_meshes.size();
		serializer << CppSer::Pair::BeginTab;
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			serializer << CppSer::Pair::Key << "Mesh " + std::to_string(i) << CppSer::Pair::Value << m_meshes[i].lock()->GetFileInfo().GetFileName();
		}
		serializer << CppSer::Pair::EndTab;
		serializer << CppSer::Pair::EndMap << "Model";
	}

	void Resource::Model::Deserialize(CppSer::Parser& parser)
	{
		IResource::Deserialize(parser);
		parser.PushDepth();
		const size_t meshCount = parser["Mesh Count"].As<size_t>();
		for (int i = 0; i < meshCount; i++)
		{
			std::string meshName = parser["Mesh " + std::to_string(i)];
			auto meshPath = Mesh::CreateMeshPath(GetFileInfo().GetFullPath(), meshName);
			Resource::ResourceManager::AddResource<Mesh>(meshPath);
		}
	}

	void Resource::Model::OnMeshLoaded()
	{
		for (auto& mesh : m_meshes)
		{
			if (!mesh.lock()->HasBeenSent())
				return;
		}
		p_loaded.store(true);
		p_hasBeenSent.store(true);
		OnLoad.Invoke();

		if (Editor::ThumbnailCreator::IsThumbnailUpToDate(this))
			return;
		//CreateThumbnail();
	}

	void Resource::Model::CreateThumbnail()
	{
		Editor::ThumbnailCreator* thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();

		const Weak modelWeak = std::dynamic_pointer_cast<Model>(shared_from_this());

		thumbnailCreator->AddToQueue(modelWeak);
	}

	void Resource::Model::ComputeBoundingBox(const std::vector<std::vector<Vec3f>>& positionVertices)
	{
		
		for (size_t i = 0; auto& weakMesh : m_meshes)
		{
			const Shared<Mesh> mesh = weakMesh.lock();
			mesh->ComputeBoundingBox(positionVertices[i]);

			m_boundingBox.min.x = std::min(m_boundingBox.min.x, mesh->m_boundingBox.min.x);
			m_boundingBox.min.y = std::min(m_boundingBox.min.y, mesh->m_boundingBox.min.y);
			m_boundingBox.min.z = std::min(m_boundingBox.min.z, mesh->m_boundingBox.min.z);

			m_boundingBox.max.x = std::max(m_boundingBox.max.x, mesh->m_boundingBox.max.x);
			m_boundingBox.max.y = std::max(m_boundingBox.max.y, mesh->m_boundingBox.max.y);
			m_boundingBox.max.z = std::max(m_boundingBox.max.z, mesh->m_boundingBox.max.z);
			i++;
		}

		m_boundingBox.center = (m_boundingBox.min + m_boundingBox.max) / 2.0f;
	}

}