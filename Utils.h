#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

#ifdef _MSC_VER 
#include <locale>
#endif

class FileUtils
{
public:
	static bool IsFileExist(const std::string& fileName);
	static bool IsFileWritable(const std::string& fileName);
	static bool IsFileEmpty(std::string& filePath);
	static bool IsFolder(const std::string& path);
	static std::string GetFileExtension(const std::string& fileName);
	static std::vector<std::string> GetFilesInFolder(const std::string& path);
	static std::string GetTempFolder();
	static void RemoveFile(const std::string& fName);
	static size_t GetFileSize(const std::string& fName);
	static void GetFolderOfFile(const std::string& fName, std::string& folder);
	static void GetExecutableFolder(std::string& folder);
	static bool CreateFolder(const std::string& fullPath);
	static void GetFileName(const std::string& fullPath, std::string& fName, bool keepExtension);
	static bool Write(const std::string& fullPath, std::string text);
	static bool Copy(const std::string& from, const std::string& to);
	static std::string ReadAll(std::string fName, bool* ok = nullptr);
	static std::string WriteToTempFile(const std::string& text, const std::string fName);

#ifdef _MSC_VER // wchar stuff
	static bool IsFileExist(const std::wstring& fileName);
	static bool IsFileWritable(const std::wstring& fileName);
	static void RemoveFile(const std::wstring& fName);
	static void GetFolderOfFile(const std::wstring& fName, std::wstring& folder);
	
	static void GetExecutableFolder(std::wstring& folder);
#endif
};

class StringUtils 
{
public:
	static std::vector<std::string> Split(const std::string& str, char delim, bool keepEmptyParts = true);
	static std::vector<std::string> Split(const std::string& str, const std::string& delim, bool keepEmptyParts = true);
	static std::string Join(const std::vector<std::string>& strings, char delim, bool keepEmptyParts = true);
	static std::string Trim(const std::string& s);
	static std::string TrimLeft(const std::string& s);
	static std::string TrimRight(const std::string& s);
	static std::string Replace(const std::string& str, const std::string& what, const std::string& with);

#ifdef _MSC_VER // wchar stuff
	static std::string ToNarrow(const wchar_t *s, char dfault = '?', const std::locale& loc = std::locale());
	static std::wstring ToWide(const char* s);
#endif
};

#include <chrono>
class Stopwatch {
public:
	Stopwatch(std::string messageName);
	~Stopwatch();

private:
	std::string msg;
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point end;
};
#endif