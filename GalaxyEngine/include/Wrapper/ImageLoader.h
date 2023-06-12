#include "GalaxyAPI.h"
namespace GALAXY::Wrapper::ImageLoader {

	void FlipVerticalOnLoad(bool flagTrueIfShouldFlip);
	unsigned char* Load(char const* filename, int* x, int* y, int* comp, int req_comp);
	void ImageFree(void* free);

}
