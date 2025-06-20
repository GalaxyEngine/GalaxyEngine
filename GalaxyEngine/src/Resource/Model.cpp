#include "pch.h"

#include "Render/Camera.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/ResourceManager.h"

#include "Component/MeshComponent.h"

#include "Core/Application.h"

#ifdef WITH_EDITOR
#include "Editor/ThumbnailCreator.h"
#endif

#include "Physic/Plane.h"
#include "Physic/AABB.h"

#include "Wrapper/OBJLoader.h"
#include "Wrapper/FBXLoader.h"

namespace GALAXY {
	Resource::BoundingBox::BoundingBox(const Physic::AABB& aabb)
	{
		min = aabb.Min;
		max = aabb.Max;
	}

	Vec3f Resource::BoundingBox::GetCenter() const
	{
		return (min + max) * 0.5f;
	}

	Vec3f Resource::BoundingBox::GetExtents() const
	{
		return max - GetCenter();
	}

	bool Resource::BoundingBox::IsOnFrustum(Render::Camera* camera, const Component::Transform* objectTransform) const
	{
		auto frustum = camera->GetFrustum();

		Vec3f position = objectTransform->GetWorldPosition();
		Quat rotation = objectTransform->GetWorldRotation();
		Vec3f scale = objectTransform->GetWorldScale();

		Vec3f localCenter = (min + max) * 0.5f;
		Vec3f localExtents = (max - min) * 0.5f;

		Vec3f worldCenter = position + rotation * (localCenter * scale);

		Vec3f worldExtents(0.0f);
		for (int i = 0; i < 8; i++)
		{
			Vec3f corner;
			corner.x = (i & 0x1) ? localExtents.x : -localExtents.x;
			corner.y = (i & 0x2) ? localExtents.y : -localExtents.y;
			corner.z = (i & 0x4) ? localExtents.z : -localExtents.z;

			corner = rotation * (corner * scale);

			for (int j = 0; j < 3; j++)
			{
				worldExtents[j] = fmaxf(worldExtents[j], fabsf(corner[j]));
			}
		}

		BoundingBox globalAABB = Physic::AABB(worldCenter, worldExtents, true);
    
		// Wrapper::Renderer::GetInstance()->DrawWireCube(globalAABB.GetCenter(), globalAABB.GetExtents(), Vec4f(1, 0, 0, 1), 5.f);

		// Check if the global AABB is on or in front of each frustum plane.
		for (const auto& plane : frustum.planes)
		{
			if (!globalAABB.isOnOrForwardPlane(plane))
			{
				return false;
			}
		}
		return true;
	}


	bool Resource::BoundingBox::isOnOrForwardPlane(const Physic::Plane& plane) const
	{
		const auto center = GetCenter();
		const auto extents = GetExtents();

		const float r = 
			  extents.x * std::abs(plane.normal.x) 
			+ extents.y * std::abs(plane.normal.y) 
			+ extents.z * std::abs(plane.normal.z);

		const float distance = plane.GetDistanceToPlane(center);

		return -r <= distance;
	}

	Resource::Model::~Model()
	{
		m_meshes.clear();
		m_meshes.shrink_to_fit();
	}

	bool Resource::Model::Load()
	{		
		if (p_fileInfo.GetExtension() == ".fbx")
		{
			m_modelType = Resource::ModelExtension::FBX;
			Wrapper::FBXLoader::Load(p_fileInfo.GetFullPath(), this);
		}
		else if (p_fileInfo.GetExtension() == ".obj")
		{
			m_modelType = Resource::ModelExtension::OBJ;
			Wrapper::OBJLoader::Load(p_fileInfo.GetFullPath(), this);
		}

		// Call every time because meshes can change
		CreateDataFile();
#ifdef WITH_EDITOR
		if (!Editor::ThumbnailCreator::IsThumbnailUpToDate(this))
			CreateThumbnail();
		for (Weak<Mesh>& mesh : m_meshes)
		{
			if (!Editor::ThumbnailCreator::IsThumbnailUpToDate(mesh.lock().get()))
				mesh.lock()->CreateThumbnail();
		}
#endif
		return true;
	}

	void Resource::Model::Unload()
	{
		for (auto& mesh : m_meshes)
		{
			ResourceManager::RemoveResource(mesh.lock()->GetFileInfo().GetFullPath());
		}
	}

	void Resource::Model::Send()
	{
		if (!m_meshesAdded)
			return;
		for (auto& mesh : m_meshes)
		{
			if (!mesh.lock()->HasBeenSent())
				return;
		}
		p_loaded.store(true);
		p_hasBeenSent.store(true);
	}

#ifdef WITH_EDITOR
	Path Resource::Model::GetThumbnailPath() const
	{
		return Editor::ThumbnailCreator::GetThumbnailPath(this);
	}
#endif
	

	Shared<Core::GameObject> Resource::Model::ToGameObject()
	{
		Shared<Core::GameObject> root = std::make_shared<Core::GameObject>(GetFileInfo().GetFileNameNoExtension());
		size_t materialIndex = 0;
		for (auto& mesh : m_meshes)
		{
			Shared<Core::GameObject> meshGO = std::make_shared<Core::GameObject>(mesh.lock()->GetMeshName());
			Weak<Component::MeshComponent> meshComponent = meshGO->AddComponent<Component::MeshComponent>();
			meshComponent.lock()->SetMesh(mesh);
			for ([[maybe_unused]] auto& subMesh : mesh.lock()->m_subMeshes) {
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
		serializer << CppSer::Pair::BeginTab;
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			auto mesh = m_meshes[i].lock();
			auto meshName = mesh->GetFileInfo().GetFileName();
			meshName = meshName.substr(meshName.find(':') + 1);
			serializer << CppSer::Pair::Key << mesh->GetUUID() << CppSer::Pair::Value << meshName;
		}
		serializer << CppSer::Pair::EndTab;
		serializer << CppSer::Pair::EndMap << "Model";
	}

	void Resource::Model::Deserialize(CppSer::Parser& parser)
	{
		IResource::Deserialize(parser);
		parser.PushDepth();
		const size_t meshCount = parser["Mesh Count"].As<size_t>();
		if (meshCount != std::numeric_limits<size_t>::max())
		{
			PrintError("You have a out dated .gdata file");
			return;
		}
		for (auto [key, value] : parser.GetValueMap()[parser.GetCurrentDepth()])
		{
			auto meshPath = Mesh::CreateMeshPath(p_fileInfo.GetFullPath(), value.As<std::string>());
			auto mesh = Resource::ResourceManager::AddResource<Mesh>(meshPath).lock();
			Core::UUID uuid = std::stoull(key);
			mesh->SetUUID(uuid);
		}
	}

#ifdef WITH_EDITOR
	void Resource::Model::ShowInInspector()
	{
		if (ImGui::Button("Reload Thumbnail"))
		{
			CreateThumbnail();
		}
	}
#endif

#ifdef WITH_EDITOR
	void Resource::Model::CreateThumbnail()
	{
		Editor::ThumbnailCreator* thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();

		const Weak modelWeak = std::dynamic_pointer_cast<Model>(shared_from_this());

		thumbnailCreator->AddToQueue(modelWeak);
	}
#endif

	void Resource::Model::DrawBoundingBox(const Component::Transform* transform) const
	{
		const BoundingBox box = GetBoundingBox();
		const auto instance = Wrapper::Renderer::GetInstance();

		instance->DrawWireCube(transform->GetWorldPosition() + box.GetCenter(), Vec3f(
			(box.max.x - box.min.x) / 2.0f,
			(box.max.y - box.min.y) / 2.0f,
			(box.max.z - box.min.z) / 2.0f
		) * transform->GetWorldScale(), Vec4f(1, 0, 0, 1), 5.f);
	}

	List<Weak<Resource::Material>> Resource::Model::GetMaterialsOfMesh(const Mesh* mesh) const
	{
		List<Weak<Resource::Material>> materials;
		size_t index = 0;
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			if (m_meshes[i].lock().get() == mesh)
			{
				for (size_t j = 0; j < mesh->m_subMeshes.size(); j++)
				{
					if (m_materials.size() > index + j)
						materials.push_back(m_materials[index + j]);
					else
						materials.push_back(ResourceManager::GetDefaultMaterial());
				}
			}
			index += m_meshes[i].lock()->m_subMeshes.size();
		}
		return materials;
	}

	void Resource::Model::ComputeBoundingBox(const std::vector<std::vector<Vec3f>>& positionVertices)
	{
		if (positionVertices.empty())
			return;
		for (size_t i = 0; auto & weakMesh : m_meshes)
		{
			const Shared<Mesh> mesh = weakMesh.lock();
			if (!mesh)
				continue;
			mesh->ComputeBoundingBox(positionVertices[i]);

			m_boundingBox.min.x = std::min(m_boundingBox.min.x, mesh->m_boundingBox.min.x);
			m_boundingBox.min.y = std::min(m_boundingBox.min.y, mesh->m_boundingBox.min.y);
			m_boundingBox.min.z = std::min(m_boundingBox.min.z, mesh->m_boundingBox.min.z);

			m_boundingBox.max.x = std::max(m_boundingBox.max.x, mesh->m_boundingBox.max.x);
			m_boundingBox.max.y = std::max(m_boundingBox.max.y, mesh->m_boundingBox.max.y);
			m_boundingBox.max.z = std::max(m_boundingBox.max.z, mesh->m_boundingBox.max.z);
			i++;
		}
	}
}