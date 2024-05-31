#include "GalaxyAPI.h"
#include <galaxymath/Maths.h>

namespace GALAXY::Wrapper
{
	struct Image
	{
		Vec2i size;
		unsigned char* data;
	};
	namespace ImageLoader {

		void FlipVerticalOnLoad(bool flagTrueIfShouldFlip);

		Image Load(char const* filename, int req_comp);
		Image* LoadIco(char const* filename, int* count);

		Image LoadFromMemory(unsigned char* data, int len);

		void SaveImage(const char* filename, const Image& image);

		void ImageFree(void* data);
		void ImageFree(const Image& image);

		// This will devided a cubemap texture to 6 different textures saved inside the same folder.
		//     +---+
		//     | U |
		// +---+---+---+---+
		// | L | F | R | B |
		// +---+---+---+---+
		//     | D |
		//     +---+
		void CubemapTextureToSixSided(const std::filesystem::path& cubemapPath);

		
	}
}