#include "pch.h"
#include "Wrapper/ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "Resource/Cubemap.h"

namespace GALAXY {
	void Wrapper::ImageLoader::FlipVerticalOnLoad(const bool flagTrueIfShouldFlip)
	{
		stbi_set_flip_vertically_on_load(flagTrueIfShouldFlip);
	}

	void Wrapper::ImageLoader::ImageFree(void* data)
	{
		stbi_image_free(data);
	}

	void Wrapper::ImageLoader::ImageFree(const Image& image)
	{
		stbi_image_free(image.data);
	}

	std::array<Wrapper::Image, 6> Wrapper::ImageLoader::CubemapTextureToSixSided(const Wrapper::Image& cubemapTexture)
	{
		std::array<Wrapper::Image, 6> output;
		
		int faceWidth = cubemapTexture.size.x / 4;
		int faceHeight = cubemapTexture.size.y / 3;

		// Extract each face
		output[0] = ExtractFace(cubemapTexture, faceWidth * 2, faceHeight * 1, faceWidth, faceHeight); // Right
		output[1] = ExtractFace(cubemapTexture, 0, faceHeight * 1, faceWidth, faceHeight); // Left
		output[2] = ExtractFace(cubemapTexture, faceWidth * 1, 0, faceWidth, faceHeight); // Up
		output[3] = ExtractFace(cubemapTexture, faceWidth * 1, faceHeight * 2, faceWidth, faceHeight); // Down
		output[4] = ExtractFace(cubemapTexture, faceWidth * 1, faceHeight * 1, faceWidth, faceHeight); // Front
		output[5] = ExtractFace(cubemapTexture, faceWidth * 3, faceHeight * 1, faceWidth, faceHeight); // Back

		return output;
	}

	Wrapper::Image Wrapper::ImageLoader::ExtractFace(const Wrapper::Image& cubemap, int x, int y, int faceWidth, int faceHeight)
	{
		Wrapper::Image face;
		face.size.x = faceWidth;
		face.size.y = faceHeight;
		face.data = new unsigned char[faceWidth * faceHeight * cubemap.channels];

		for (int j = 0; j < faceHeight; ++j) {
			for (int i = 0; i < faceWidth; ++i) {
				for (int k = 0; k < cubemap.channels; ++k) {
					face.data[(j * faceWidth + i) * cubemap.channels + k] =
						cubemap.data[((y + j) * cubemap.size.x + (x + i)) * cubemap.channels + k];
				}
			}
		}

		return face;
	}

	void Wrapper::ImageLoader::ExtractSixSidedFromCubemap(const std::filesystem::path& cubemapPath)
	{
		stbi_set_flip_vertically_on_load(0);

		Wrapper::Image cubemap = Load(cubemapPath.string().c_str(), 4);

		auto faces = CubemapTextureToSixSided(cubemap);

		// Save each face
		for (int i = 0; i < 6; ++i) {
			auto directionString = Resource::Cubemap::GetDirectionFromIndex(i);
			std::string filename = cubemapPath.parent_path().string() + "/" + directionString + ".png";
			SaveImage(filename.c_str(), faces[i]);
			ImageFree(faces[i]);
		}
		
		ImageFree(cubemap);
	}

	Wrapper::Image Wrapper::ImageLoader::Load(char const* filename, int req_comp)
	{
		int x, y, comp;
		unsigned char* data = stbi_load(filename, &x, &y, &comp, req_comp);

		Image image;
		image.data = data;
		image.size.x = x;
		image.size.y = y;
		image.channels = req_comp;
		return image;
	}

	Wrapper::Image Wrapper::ImageLoader::Load(const std::filesystem::path& path, int req_comp)
	{
		return Load(path.generic_string().c_str(), req_comp);
	}

	void Wrapper::ImageLoader::SaveImage(const char* filename, const Image& image)
	{
		stbi_write_png(filename, image.size.x, image.size.y, 4, image.data, 4 * image.size.x);
	}

	Wrapper::Image Wrapper::ImageLoader::LoadFromMemory(unsigned char* data, int len)
	{
		Image image;
		int comp;
		image.data = stbi_load_from_memory(data, len, &image.size.x, &image.size.y, &comp, 4);

		return image;
	}

}
