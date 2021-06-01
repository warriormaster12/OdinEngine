#pragma once 

#include <iostream>
#include <string>

#include "vk_types.h"
#include "unordered_finder.h"

struct Texture {
	AllocatedImage image;
	VkImageView imageView;

	void CreateTexture(const std::string& filePath);
	void CreateCubeMapTexture(const std::vector<std::string>& filepaths);
	void DestroyTexture();
};