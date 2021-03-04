#include "json.hpp"


#include "lz4.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include<glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Include/asset_builder.h"
#include <fstream>
#include "logger.h"


using namespace assets;


bool asset_builder::ConvertImage(const fs::path& input, const fs::path& output, assets::TextureInfo& texFormatInfo)
{
	int texWidth, texHeight, texChannels;
	std::string binary_filename = output.u8string();
    std::ifstream file_binary(binary_filename);
	if(file_binary.fail())
	{
		if(input == "" || output == ".bin")
		{
			ENGINE_CORE_INFO("path to a file is null");
			return false;
		}
		else
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
			texinfo.textureFormat = texFormatInfo.textureFormat;	
			texinfo.originalFile = input.string();
			AssetFile newImage = assets::PackTexture(&texinfo, pixels);	
				

			stbi_image_free(pixels);

			save_binaryfile(output.string().c_str(), newImage);

			return true;
		}
		
	}
	else
	{
		ENGINE_CORE_INFO("File {0} already exists", binary_filename);
		return false;
	}
}

