#include "pch.h"

#include "Wrapper/OBJLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "Core/Application.h"

void Wrapper::OBJLoader::Load(const std::filesystem::path& fullPath, Resource::Model* outputModel)
{
	//TODO
	OBJLoader model;
	model.m_path = fullPath;
	if (!model.Parse())
		return;
	for (int i = 0; i < model.m_meshes.size(); i++) {

		const std::filesystem::path& meshFullPath = Resource::Mesh::CreateMeshPath(fullPath, model.m_meshes[i].name);

		Weak<Resource::Mesh> meshWeak = Resource::ResourceManager::GetInstance()->GetResource<Resource::Mesh>(meshFullPath);
		Resource::Mesh* mesh = meshWeak.lock().get();
		if (!meshWeak.lock()) {
			// If mesh not in resource manager
			auto sharedMesh = std::make_shared<Resource::Mesh>(meshFullPath);
			Resource::ResourceManager::GetInstance()->AddResource(sharedMesh);
			mesh = sharedMesh.get();
			meshWeak = sharedMesh;
		}

		mesh->m_positions = model.m_meshes[i].positions;
		mesh->m_textureUVs = model.m_meshes[i].textureUVs;
		mesh->m_normals = model.m_meshes[i].normals;
		mesh->m_indices = model.m_meshes[i].indices;
		mesh->m_finalVertices = model.m_meshes[i].finalVertices;
		for (int j = 0; j < model.m_meshes[i].subMeshes.size(); j++) {
			Resource::SubMesh subMesh;
			subMesh.startIndex = model.m_meshes[i].subMeshes[j].startIndex;
			subMesh.count = model.m_meshes[i].subMeshes[j].count;
			mesh->m_subMeshes.push_back(subMesh);
		}

		mesh->p_shouldBeLoaded = true;
		mesh->p_loaded = true;

		outputModel->m_meshes.push_back(meshWeak);

		mesh->SendRequest();
	}
	PrintLog("Successfully Loaded Model %s", fullPath.string().c_str());
}

bool Wrapper::OBJLoader::Parse()
{
	// TODO support :
	// color
	// mtl
	// submeshes
	std::ifstream file(m_path);
	if (!file.is_open()) {
		PrintError("Failed to open OBJ file %s", m_path.string().c_str());
		return false;
	}

	auto endSubMesh = [this](OBJMesh& mesh) {
		std::vector<OBJSubMesh>& subMeshes = mesh.subMeshes;
		if (subMeshes.size() > 0)
			subMeshes.back().count = mesh.indices.size() - subMeshes.back().startIndex;
		};

	bool quadOBJ = false;
	OBJMesh currentMesh;
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "o" || token == "g")
		{
			endSubMesh(currentMesh);
			if (!currentMesh.name.empty()) {
				m_meshes.push_back(currentMesh);
			}
			currentMesh = OBJMesh();
			iss >> currentMesh.name;
		}
		if (token == "mtllib")
		{
			std::filesystem::path mtlPath;
			iss >> mtlPath;
			mtlPath = m_path.parent_path() / mtlPath;
			ReadMtl(mtlPath);
		}
		if (token == "usemtl")
		{
			endSubMesh(currentMesh);
			OBJSubMesh subMesh = OBJSubMesh();
			subMesh.startIndex = currentMesh.indices.size();

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
			std::string indexStr;
			while (iss >> indexStr) {
				Vec3i indices = ParseFaceIndex(indexStr);
				currentMesh.indices.push_back(indices);
			}

			// This will happen only one time by mesh, check if it's a quad model
			if (currentMesh.indices.size() == 4 && !quadOBJ)
			{
				quadOBJ = true;
			}
		}
	}
	if (!currentMesh.name.empty()) {
		endSubMesh(currentMesh);
		m_meshes.push_back(currentMesh);
	}

	// Convert to Triangulate faces if it's a quad model
	if (quadOBJ)
	{
		std::vector<Vec3i> triangulateFace;

		for (size_t i = 0; i < m_meshes.size(); i++) {
			ConvertQuadToTriangles(m_meshes[i].indices, triangulateFace);
			m_meshes[i].indices = triangulateFace;
		}
	}


	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		ComputeVertices(m_meshes[i]);
	}

	return true;
}

Vec3i Wrapper::OBJLoader::ParseFaceIndex(const std::string& indexStr)
{
	Vec3i indices;
	std::istringstream indexStream(indexStr);
	std::string vertexIndexStr, uvIndexStr, normalIndexStr;

	std::getline(indexStream, vertexIndexStr, '/');
	std::getline(indexStream, uvIndexStr, '/');
	std::getline(indexStream, normalIndexStr, '/');

	indices.x = std::stoi(vertexIndexStr) - 1;   // OBJ indices start from 1
	// Case when no uvs
	try
	{
		indices.y = std::stoi(uvIndexStr) - 1;
	}
	catch (const std::exception&)
	{
		indices.y = -1;
	}
	// Case when no normals
	try
	{
		indices.z = std::stoi(normalIndexStr) - 1;
	}
	catch (const std::exception&)
	{
		indices.z = -1;
	}
	return indices;
}

bool AreVerticesSimilar(const Vec3f& v1, const Vec2f& uv1, const Vec3f& n1,
	const Vec3f& v2, const Vec2f& uv2, const Vec3f& n2)
{
	const float epsilon = 0.0001f;
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
	}
}

void Wrapper::OBJLoader::ConvertQuadToTriangles(const std::vector<Vec3i>& quadIndices, std::vector<Vec3i>& triangleIndices)
{

	if (quadIndices.size() % 4 != 0) {
		PrintError("The number of index is not valid");
		return;
	}

	const size_t numQuads = quadIndices.size() / 4;

	for (int i = 0; i < numQuads; ++i) {
		const int quadOffset = i * 4;

		// Convertit chaque quad en deux triangles
		triangleIndices.push_back(quadIndices[quadOffset]);
		triangleIndices.push_back(quadIndices[quadOffset + 1]);
		triangleIndices.push_back(quadIndices[quadOffset + 2]);

		triangleIndices.push_back(quadIndices[quadOffset]);
		triangleIndices.push_back(quadIndices[quadOffset + 2]);
		triangleIndices.push_back(quadIndices[quadOffset + 3]);
	}
}

bool Wrapper::OBJLoader::ReadMtl(const std::filesystem::path& mtlPath)
{
	std::ifstream file(mtlPath);
	if (!file.is_open()) {
		PrintError("Failed to open MTL file %s", mtlPath.c_str());
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
			const std::filesystem::path& matFullPath = mtlPath.parent_path() / name.append(".mat");
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
			texPath = mtlPath.parent_path() / texPath;
			currentMaterial->m_albedo = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(texPath);
		}
	}
	endMaterial(currentMaterial);
	return true;
}
