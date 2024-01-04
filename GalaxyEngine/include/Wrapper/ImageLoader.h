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

		Wrapper::Image Load(char const* filename, int req_comp);
		Wrapper::Image* LoadIco(char const* filename, int* count);

		void ImageFree(void* data);
		void ImageFree(const Image& image);

	}
}