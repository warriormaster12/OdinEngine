#include "../Third-Party/nlohmann_json/json.hpp"


#include "../Third-Party/lz4/Include/lz4.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "../Third-Party/stb_image/stb_image.h"

#include "Include/texture_asset.h"

#include <glm/glm.hpp>
#include<glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Include/asset_builder.h"
#include <fstream>
#include "../Logger/Include/logger.h"


using namespace assets;


bool asset_builder::convert_image(const fs::path& input, const fs::path& output)
{
	int texWidth, texHeight, texChannels;
	std::string binary_filename = output.u8string();
    std::ifstream file_binary(binary_filename);
	if(file_binary.fail())
	{
		stbi_uc* pixels = stbi_load(input.u8string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels) {
			ENGINE_CORE_ERROR("Failed to load texture file {0}", input);
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
	else
	{
		ENGINE_CORE_INFO("File {0} already exists", binary_filename);
		return false;
	}
}

