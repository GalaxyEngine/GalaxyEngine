#include "pch.h"
#include "Wrapper/ImageLoader.h"

unsigned char* Wrapper::ImageLoader::Load(char const* filename, int* x, int* y, int* comp, int req_comp)
{
	stbi_load(filename, x, y, comp, req_comp);
}

void Wrapper::ImageLoader::FlipVerticalOnLoad(bool flagTrueIfShouldFlip)
{
	stbi_set_flip_vertically_on_load(flagTrueIfShouldFlip);
}

void Wrapper::ImageLoader::ImageFree(void* free)
{
	stbi_image_free(free);
}
