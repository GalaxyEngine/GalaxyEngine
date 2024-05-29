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

		void CubemapTextureToSixSided(const std::filesystem::path& cubemapPath);

	}
}