#pragma once 
#include <fstream>
#include <filesystem>

#include "texture_asset.h"

#include <iostream>

namespace fs = std::filesystem;

struct ConverterState {
	fs::path asset_path;
	fs::path export_path;

	fs::path convert_to_export_relative(fs::path path)const;
};

namespace asset_builder
{
    bool ConvertImage(const fs::path& input, const fs::path& output, assets::TextureInfo& texFormatInfo);
}

