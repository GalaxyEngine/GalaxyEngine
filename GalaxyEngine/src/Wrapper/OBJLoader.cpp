#include "pch.h"

#include "Wrapper/OBJLoader.h"
#include "Wrapper/MTLLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "Core/Application.h"

void Wrapper::OBJLoader::Load(const std::filesystem::path& fullPath, Resource::Model* outputModel)
{
	PROFILE_SCOPE_LOG("OBJLoader::Load(%s)", fullPath.string().c_str());
	
	OBJLoader model;
	model.m_path = fullPath;
	if (!model.Parse())
		return;
	std::vector<std::vector<Vec3f>> positionVertices;
	positionVertices.resize(model.m_meshes.size());
	for (size_t i = 0; i < model.m_meshes.size(); i++) {

		const std::filesystem::path& meshFullPath = Resource::Mesh::CreateMeshPath(fullPath, model.m_meshes[i].name);

		Weak<Resource::Mesh> meshWeak = Resource::ResourceManager::GetInstance()->GetResource<Resource::Mesh>(meshFullPath);
		Resource::Mesh* mesh = meshWeak.lock().get();
		if (!meshWeak.lock()) {
			// If mesh not in resource manager
			auto sharedMesh = Resource::ResourceManager::AddResource<Resource::Mesh>(meshFullPath).lock();
			mesh = sharedMesh.get();
			meshWeak = sharedMesh;
		}

		positionVertices[i] = model.m_meshes[i].positions;
		mesh->m_indices = model.m_meshes[i].indices;
		mesh->m_finalVertices = model.m_meshes[i].finalVertices;
		for (size_t j = 0; j < model.m_meshes[i].subMeshes.size(); j++) {
			Resource::SubMesh subMesh;
			subMesh.startIndex = model.m_meshes[i].subMeshes[j].startIndex;
			subMesh.count = model.m_meshes[i].subMeshes[j].count;
			mesh->m_subMeshes.push_back(subMesh);
			if (model.m_mtlPath.has_value() && model.m_meshes[i].subMeshes[j].material.has_value()) {
				const std::string materialName = model.m_meshes[i].subMeshes[j].material->name.generic_string();
				std::filesystem::path materialPath = MTLLoader::GetMaterialPath(model.m_mtlPath.value(), materialName);
				auto material = Resource::ResourceManager::GetInstance()->GetResource<Resource::Material>(materialPath);
				outputModel->m_materials.push_back(material);
			}
		}

		mesh->p_shouldBeLoaded = true;
		mesh->p_loaded = true;

		outputModel->m_meshes.push_back(meshWeak);

		mesh->m_model = outputModel;

		auto bind = [outputModel] { outputModel->OnMeshLoaded(); };
		mesh->OnLoad.Bind(bind);

		mesh->SendRequest();
	}
	outputModel->p_hasBeenSent = true;
	outputModel->ComputeBoundingBox(positionVertices);

	PrintLog("Successfully Loaded Model %s", fullPath.string().c_str());
}

bool Wrapper::OBJLoader::Parse()
{
	std::ifstream file(m_path);
	if (!file.is_open()) {
		PrintError("Failed to open OBJ file %s", m_path.string().c_str());
		return false;
	}


	auto endSubMesh = [&](OBJMesh& mesh) {
		std::vector<OBJSubMesh>& subMeshes = mesh.subMeshes;
		if (subMeshes.size() > 0) {
			subMeshes.back().count = mesh.indices.size() - subMeshes.back().startIndex;
		}
		};

	OBJMesh currentMesh;
	std::string line;
	Vec3i lastSize = {};
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "o" || token == "g")
		{
			lastSize = lastSize + Vec3i{ (int)currentMesh.positions.size(), (int)currentMesh.textureUVs.size(), (int)currentMesh.normals.size() };
			endSubMesh(currentMesh);
			if (!currentMesh.name.empty()) {
				m_meshes.push_back(currentMesh);
			}
			currentMesh = OBJMesh();
			iss >> currentMesh.name;
		}
		if (token == "mtllib")
		{
			std::string mtlFileName;
			iss >> mtlFileName;
			m_mtlPath = m_path.parent_path() / mtlFileName;
			ReadMtl(m_mtlPath.value());
		}
		if (token == "usemtl")
		{
			endSubMesh(currentMesh);
			std::string materialName;
			iss >> materialName;
			OBJSubMesh subMesh = OBJSubMesh();
			subMesh.startIndex = currentMesh.indices.size();
			subMesh.material.emplace(OBJMaterial{ materialName });

			currentMesh.subMeshes.push_back(subMesh);
		}
		if (token == "v")
		{
			Vec3f position;
			iss >> position.x >> position.y >> position.z;
			currentMesh.positions.push_back(position);
		}
		else if (token == "vt")
		{
			Vec2f uv;
			iss >> uv.x >> uv.y;
			uv.y = 1 - uv.y;
			currentMesh.textureUVs.push_back(uv);
		}
		else if (token == "vn")
		{
			Vec3f normal;
			iss >> normal.x >> normal.y >> normal.z;

			currentMesh.normals.push_back(normal);
		}
		else if (token == "f")
		{
			if (currentMesh.subMeshes.empty())
			{
				OBJSubMesh subMesh = OBJSubMesh();
				subMesh.startIndex = currentMesh.indices.size();
				currentMesh.subMeshes.push_back(subMesh);
			}

			size_t count = 0;
			std::string indexStr;
			while (iss >> indexStr) {
				Vec3i indices;
				ParseFaceIndex(std::ref(indices), indexStr);
				currentMesh.indices.push_back(indices - lastSize);
				count++;
			}

			if (count == 4)
			{
				size_t lastIndex = currentMesh.indices.size() - 1;
				Vec3i i1 = currentMesh.indices[lastIndex - 3];
				Vec3i i2 = currentMesh.indices[lastIndex - 2];
				Vec3i i3 = currentMesh.indices[lastIndex - 1];
				Vec3i i4 = currentMesh.indices[lastIndex];

				// Remove the quad indices
				currentMesh.indices.pop_back();
				currentMesh.indices.pop_back();
				currentMesh.indices.pop_back();
				currentMesh.indices.pop_back();

				// Push the first triangle indices
				currentMesh.indices.push_back(i1);
				currentMesh.indices.push_back(i2);
				currentMesh.indices.push_back(i3);

				// Push the second triangle indices
				currentMesh.indices.push_back(i1);
				currentMesh.indices.push_back(i3);
				currentMesh.indices.push_back(i4);
			}
		}
	}
	if (!currentMesh.name.empty()) {
		endSubMesh(currentMesh);
		m_meshes.push_back(currentMesh);
	}

	for (auto& m_mesh : m_meshes)
	{
		ComputeVertices(m_mesh);
	}

	return true;
}

void Wrapper::OBJLoader::ParseFaceIndex(Vec3i& indices, const std::string& indexStr)
{
	size_t firstSlash = indexStr.find('/');
	size_t secondSlash = indexStr.find('/', firstSlash + 1);

	// Vertex index
	if (firstSlash != std::string::npos) {
		indices.x = std::stoi(indexStr.substr(0, firstSlash)) - 1; // OBJ indices start from 1
	}

	// Texture index
	if (firstSlash != std::string::npos && secondSlash != std::string::npos) {
		std::string uvIndexStr = indexStr.substr(firstSlash + 1, secondSlash - firstSlash - 1);
		if (!uvIndexStr.empty()) {
			indices.y = std::stoi(uvIndexStr) - 1;
		}
	}

	// Normal index
	if (secondSlash != std::string::npos && secondSlash + 1 < indexStr.size()) {
		std::string normalIndexStr = indexStr.substr(secondSlash + 1);
		if (!normalIndexStr.empty()) {
			indices.z = std::stoi(normalIndexStr) - 1;
		}
	}
}

bool AreVerticesSimilar(const Vec3f& v1, const Vec2f& uv1, const Vec3f& n1,
	const Vec3f& v2, const Vec2f& uv2, const Vec3f& n2)
{
	constexpr float epsilon = 0.0001f;
	return (std::abs(v1.x - v2.x) < epsilon &&
		std::abs(v1.y - v2.y) < epsilon &&
		std::abs(v1.z - v2.z) < epsilon &&
		std::abs(uv1.x - uv2.x) < epsilon &&
		std::abs(uv1.y - uv2.y) < epsilon &&
		std::abs(n1.x - n2.x) < epsilon &&
		std::abs(n1.y - n2.y) < epsilon &&
		std::abs(n1.z - n2.z) < epsilon);
}

void Wrapper::OBJLoader::ComputeVertices(OBJMesh& mesh)
{
	mesh.tangents.resize(mesh.indices.size());

	for (size_t k = 0; k < mesh.indices.size(); k += 3)
	{
		const Vec3i& idx0 = mesh.indices[k];
		const Vec3i& idx1 = mesh.indices[k + 1];
		const Vec3i& idx2 = mesh.indices[k + 2];

		const Vec3f& Edge1 = mesh.positions[idx1.x] - mesh.positions[idx0.x];
		const Vec3f& Edge2 = mesh.positions[idx2.x] - mesh.positions[idx0.x];

		const float DeltaU1 = mesh.textureUVs[idx1.y].x - mesh.textureUVs[idx0.y].x;
		const float DeltaV1 = mesh.textureUVs[idx1.y].y - mesh.textureUVs[idx0.y].y;
		const float DeltaU2 = mesh.textureUVs[idx2.y].x - mesh.textureUVs[idx0.y].x;
		const float DeltaV2 = mesh.textureUVs[idx2.y].y - mesh.textureUVs[idx0.y].y;

		float f = DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1;
		if (fabs(f) < 1e-6) {
			f = 1.0f; // Prevent division by zero and provide a default value for 'f'
		}
		else {
			f = 1.0f / f;
		}

		Vec3f Tangent;

		Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
		Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
		Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

		Tangent.Normalize();

		mesh.tangents[k] = Tangent;
		mesh.tangents[k + 1] = Tangent;
		mesh.tangents[k + 2] = Tangent;
	}

	for (size_t i = 0; i < mesh.indices.size(); i++)
	{
		const Vec3i& idx = mesh.indices[i];

		mesh.finalVertices.push_back(mesh.positions[idx.x].x);
		mesh.finalVertices.push_back(mesh.positions[idx.x].y);
		mesh.finalVertices.push_back(mesh.positions[idx.x].z);

		mesh.finalVertices.push_back(mesh.textureUVs[idx.y].x);
		mesh.finalVertices.push_back(mesh.textureUVs[idx.y].y);

		mesh.finalVertices.push_back(mesh.normals[idx.z].x);
		mesh.finalVertices.push_back(mesh.normals[idx.z].y);
		mesh.finalVertices.push_back(mesh.normals[idx.z].z);

		mesh.finalVertices.push_back(mesh.tangents[i].x);
		mesh.finalVertices.push_back(mesh.tangents[i].y);
		mesh.finalVertices.push_back(mesh.tangents[i].z);
	}
}

bool Wrapper::OBJLoader::ReadMtl(const std::filesystem::path& mtlPath)
{
	return Wrapper::MTLLoader::Load(mtlPath);
}

Wrapper::OBJLoader::~OBJLoader()
{
	int x = 0;
	x++;
}
