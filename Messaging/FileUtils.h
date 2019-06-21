#pragma once
#include <cstdio>
#include <string>
namespace file_utils {
	inline bool FileReadAllText(const char* fileName, std::string& outText) {
		FILE* file = ::fopen(fileName, "r");
		if (file == nullptr) {
			return false;
		}
		::fseek(file, 0, SEEK_END);
		const auto Size = ::ftell(file);
		outText.resize(Size);
		::fseek(file, 0, SEEK_SET);
		::fread(outText.data(), outText.size(), 1, file);
		::fclose(file);
		return true;
	}

	inline bool FileWriteAllText(const char* fileName, const std::string& inText) {
		FILE* file = ::fopen(fileName, "w");
		if (file == nullptr) {
			return false;
		}
		::fwrite(inText.c_str(), inText.size(), 1, file);
		::fflush(file);
		::fclose(file);
		return true;
	}
}