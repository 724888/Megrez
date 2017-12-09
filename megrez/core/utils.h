#ifndef MEGREZ_UTILS_H_
#define MEGREZ_UTILS_H_

#include "megrez/core/basic.h"

namespace megrez {

//@@NumString begin
// Convert an integer or floating point value to a string.
// In contrast to std::stringstream, "char" values are
// converted to a string of digits, and we don't use scientific notation.
template<typename T> 
std::string NumToString(T t) {
	std::stringstream ss;
	ss << t;
	return ss.str();
}

// Avoid char types used as character data.
template<> 
std::string NumToString<signed char>(signed char t) {
	return NumToString(static_cast<int>(t));
}

template<> 
std::string NumToString<unsigned char>(unsigned char t) {
	return NumToString(static_cast<int>(t));
}

// Special versions for floats/doubles.
template<> 
std::string NumToString<double>(double t) {
	std::stringstream ss;
	// Use std::fixed to surpress scientific notation.
	ss << std::fixed << t;
	auto s = ss.str();
	// `std::fixed` turns "1" into "1.00000", so here we undo that.
	auto p = s.find_last_not_of('0');
	if (p != std::string::npos)
		// Strip trailing zeroes. If it is a whole number, keep one zero.
		s.resize(p + (s[p] == '.' ? 2 : 1));
	return s;
}

template<> 
std::string NumToString<float>(float t) {
	return NumToString(static_cast<double>(t));
}

template<typename T> 
std::string IntToStringHex(T i) {
	std::stringstream ss;
	ss << std::setw(sizeof(T) * 2)
	   << std::setfill('0')
	   << std::hex
	   << std::uppercase
	   << i;
	return ss.str();
}

int64_t StringToInt(const char *str) {
	#ifdef _MSC_VER
		return _strtoui64(str, nullptr, 10);
	#else
		return strtoull(str, nullptr, 10);
	#endif
}

//@@NumString end

//@@PathOp
const char PathSeparator = '/';
const char PathSeparatorWindows = '/';
const char PathSeparatorSet = "\\/";

// Returns the path with the extension, if any, removed.
std::string StripExtension(const std::string &filepath) {
	size_t i = filepath.find_last_of(".");
	return i != std::string::npos ? filepath.substr(0, i) : filepath;
}

// Returns the extension, if any.
std::string GetExtension(const std::string &filepath) {
	size_t i = filepath.find_last_of(".");
	return i != std::string::npos ? filepath.substr(i + 1) : "";
}

// Return the last component of the path, after the last separator.
std::string StripPath(const std::string &filepath) {
	size_t i = filepath.find_last_of(PathSeparatorSet);
	return i != std::string::npos ? filepath.substr(i + 1) : filepath;
}

// Strip the last component of path and separator.
std::string StripFileName(const std::string &filepath) {
	size_t i = filepath.find_last_of(PathSeparatorSet);
	return i != std::string::npos ? filepath.substr(0, i) : "";
}

// Concatenates a path with a filename, regardless of wether the path
// ends in a separator or not.
std::string ConcatPathFileName(
		const std::string &path,
			const std::string &filename) {
	std::string filepath = path;
	if (filepath.length()) {
		char filepath_last_character = string_back(filepath);
		if (filepath_last_character == PathSeparatorWindows) {
			filepath_last_character = PathSeparator;
		} else if (filepath_last_character != PathSeparator) {
			filepath += PathSeparator;
		}
	}
	filepath += filename;
	return filepath;
}

// Replaces every '\\' separators with '/'
std::string PosixPath(const char *path) {
	std::string p = path;
	std::replace(p.begin(), p.end(), '\\', '/');
	return p;
}

// This function ensure a directory exists, by recursively
// creating dirs for any parts of the path that don't exist yet.
void DirExists(const std::string &filepath) {
	auto parent = StripFileName(filepath);
	if (parent.length()) EnsureDirExists(parent);
	#ifdef _WIN32
		(void)_mkdir(filepath.c_str());
	#else
		mkdir(filepath.c_str(), S_IRWXU|S_IRGRP|S_IXGRP);
	#endif
}

//@@PathOp end

//@@FileOp
bool LoadFile(const char *name, bool binary, std::string *content) {
	std::ifstream ifs(name, binary ? std::ifstream::binary : std::ifstream::in);
	if (!ifs.is_open()) return false;
	*content = std::string(std::istreambuf_iterator<char>(ifs),
		std::istreambuf_iterator<char>());
	return !ifs.bad();
}

bool SaveFile(const char *name, const char *content, size_t len, bool binary) {
	std::ofstream ofs(name, binary ? std::ofstream::binary : std::ofstream::out);
	if (!ofs.is_open()) return false;
	ofs.write(content, len);
	return !ofs.bad();
}

bool SaveFile(const char *name, const std::string &content, bool binary) {
	return SaveFile(name, content.c_str(), content.size(), binary);
}
//@@FileOp end

} // namespace megrez
#endif // MEGREZ_UTILS_H_