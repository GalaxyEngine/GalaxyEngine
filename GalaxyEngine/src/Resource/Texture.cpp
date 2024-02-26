#include "pch.h"
#include "Resource/Texture.h"
#include "Wrapper/ImageLoader.h"
#include "Core/Application.h"

Resource::Texture::~Texture()
{
	Wrapper::Renderer::GetInstance()->DestroyTexture(this);
	if (p_shouldBeLoaded && !p_hasBeenSent)
		Wrapper::ImageLoader::ImageFree(m_bytes);
}

void Resource::Texture::Load()
{
	if (p_shouldBeLoaded)
		return;

	if (p_fileInfo.GetExtension() == ".tmb") {
		this->SetDisplayOnInspector(false);
		this->SetCreateDataFile(false);
	}

	p_shouldBeLoaded = true;
	auto image = Wrapper::ImageLoader::Load(p_fileInfo.GetFullPath().string().c_str(), 4);
	if (m_bytes = image.data) {
		p_loaded = true;
		m_size = image.size;
	}
	else
	{
		PrintError("Failed to load Image %s", p_fileInfo.GetFullPath().string().c_str());
		return;
	}

	CreateDataFile();
	SendRequest();
}

void Resource::Texture::Send()
{
	if (p_hasBeenSent)
		return;
	Wrapper::Renderer::GetInstance()->CreateTexture(this);
	Wrapper::ImageLoader::ImageFree(m_bytes);
	p_hasBeenSent = true;
}

void Resource::Texture::Bind(const uint32_t index /* = 0 */)
{
	if (!p_hasBeenSent)
		return;
	Wrapper::Renderer::GetInstance()->BindTexture(this, index);
}

void Resource::Texture::UnBind()
{
	Wrapper::Renderer::GetInstance()->UnbindTexture();
}

void Resource::Texture::Serialize(CppSer::Serializer& serializer) const
{
	IResource::Serialize(serializer);
	serializer <<CppSer::Pair::BeginMap << "Texture";
	serializer << CppSer::Pair::Key << "Filtering" << CppSer::Pair::Value << (int)m_filtering;
	serializer << CppSer::Pair::Key << "Wrapping" << CppSer::Pair::Value << (int)m_wrapping;
	serializer <<CppSer::Pair::EndMap << "Texture";
}

void Resource::Texture::Deserialize(CppSer::Parser& parser)
{
	IResource::Deserialize(parser);
	parser.PushDepth();
	m_filtering = (TextureFiltering)parser["Filtering"].As<int>();
	m_wrapping = (TextureWrapping)parser["Wrapping"].As<int>();
}

void Resource::Texture::ShowInInspector()
{
	int filteringMode = (int)m_filtering;
	if (ImGui::Combo("Filtering", &filteringMode, SerializeTextureFilteringEnum()))
	{
		Wrapper::Renderer::GetInstance()->SetTextureFiltering(this, (TextureFiltering)filteringMode);
	}

	int wrappingMode = (int)m_wrapping;
	if (ImGui::Combo("Wrapping", &wrappingMode, SerializeTextureWrappingEnum()))
	{
		Wrapper::Renderer::GetInstance()->SetTextureWrapping(this, (TextureWrapping)wrappingMode);
	}

	if (ImGui::Button("Save"))
	{
		Save();
	}
}

void Resource::Texture::Save()
{
	if (!p_loaded)
		return;

	CreateDataFile();
}
