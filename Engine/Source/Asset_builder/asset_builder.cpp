#include "../third-party/nlohmann_json/json.hpp"


#include "../third-party/lz4/Include/lz4.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "../third-party/stb_image/stb_image.h"

#include "../Asset_loader/Include/texture_asset.h"

#include <glm/glm.hpp>
#include<glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Include/asset_builder.h"



using namespace assets;


bool asset_builder::convert_image(const fs::path& input, const fs::path& output)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(input.u8string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		std::cout << "Failed to load texture file " << input << std::endl;
		return false;
	}
	
	int texture_size = texWidth * texHeight * 4;

	TextureInfo texinfo;
	texinfo.textureSize = texture_size;
	texinfo.pixelsize[0] = texWidth;
	texinfo.pixelsize[1] = texHeight;
	texinfo.textureFormat = TextureFormat::RGBA8;	
	texinfo.originalFile = input.string();
	AssetFile newImage = assets::pack_texture(&texinfo, pixels);	
		

	stbi_image_free(pixels);

	save_binaryfile(output.string().c_str(), newImage);

	return true;
}

