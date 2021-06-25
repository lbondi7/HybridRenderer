#pragma once

#include "Texture.h"

class Image
{
public:
	Image() = default;
	~Image();

	Texture texture;

	uint32_t width;
	uint32_t height;
	uint32_t channels;
};

