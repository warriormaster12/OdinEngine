#pragma once 

#include <vector>
#include <string>
#include <cstring>



namespace assets {

	enum class CompressionMode : uint32_t {
		Unknown,
		LZ4
	};
	struct AssetFile {
		char type[4];
		int version;
		std::string json;
		std::vector<char> binaryBlob;
	};	

	

	bool save_binaryfile(const char* path, const AssetFile& file);

	bool load_binaryfile(const char* path, AssetFile& outputFile);	


	CompressionMode parse_compression(const char* f);
}