#pragma once 

#include <iostream>
#include <string>

#include "vk_types.h"
#include "unordered_finder.h"
#include "renderer.h"

struct Texture {
	AllocatedImage image;

	void CreateTexture(const std::string& filePath, const ColorFormat& imageFormat);
	void CreateCubeMapTexture(const std::vector<std::string>& filepaths);
	void DestroyTexture();
};