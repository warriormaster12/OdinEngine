#pragma once 
#include <fstream>
#include <filesystem>

#include <iostream>

namespace fs = std::filesystem;

struct ConverterState {
	fs::path asset_path;
	fs::path export_path;

	fs::path convert_to_export_relative(fs::path path)const;
};

namespace asset_builder
{
    bool convert_image(const fs::path& input, const fs::path& output);
}

