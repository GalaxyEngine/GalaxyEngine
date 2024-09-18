#include "GalaxyAPI.h"
#include <galaxymath/Maths.h>

namespace GALAXY
{
	namespace Resource
	{
		class Texture;
	}
	namespace Wrapper {
		struct Image
		{
			Vec2i size;
			unsigned char* data;
			int channels;
		};
		namespace ImageLoader {

			void FlipVerticalOnLoad(bool flagTrueIfShouldFlip);

			Image Load(char const* filename, int req_comp = 4);
			Image Load(const std::filesystem::path& path, int req_comp = 4);
			Image* LoadIco(char const* filename, int* count);

			Image LoadFromMemory(unsigned char* data, int len);

			void SaveImage(const char* filename, const Image& image);

			void ImageFree(void* data);
			void ImageFree(const Image& image);

			// This will divide a cube map texture to 6 different textures saved inside the same folder.
			//     +---+
			//     | U |
			// +---+---+---+---+
			// | L | F | R | B |
			// +---+---+---+---+
			//     | D |
			//     +---+
			Wrapper::Image ExtractFace(const Wrapper::Image& cubemap, int x, int y, int faceWidth, int faceHeight);
			std::array<Wrapper::Image, 6> CubemapTextureToSixSided(const Wrapper::Image& cubemapTexture);

			/* This method save the cubemap texture to the cubemapPath
			 * @param cubemapPath : the path of the cubemap you want to extract
			 */
			void ExtractSixSidedFromCubemap(const std::filesystem::path& cubemapPath);

			Wrapper::Image FromTextureToImage(Resource::Texture* texture);
		}
	}
}