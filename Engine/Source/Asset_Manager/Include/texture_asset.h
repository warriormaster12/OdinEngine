#pragma once
#include "asset_loader.h"

namespace assets {
	enum class TextureFormat : uint32_t
	{
		Unknown = 0,
		RGBA8,
		UNORM8		
	};
	

	struct TextureInfo {
		uint64_t textureSize;
		TextureFormat textureFormat;
		CompressionMode compressionMode;
		uint32_t pixelsize[3];
		std::string originalFile;
	};

	//parses the texture metadata from an asset file
	TextureInfo ReadTextureInfo(AssetFile* file);

	void UnpackTexture(TextureInfo* info, const char* sourcebuffer, size_t sourceSize, char* destination);

	assets::AssetFile PackTexture(TextureInfo* info, void* pixelData);

}

