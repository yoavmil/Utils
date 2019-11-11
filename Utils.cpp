#include "Utils.h"
#include "stdafx.h"

using namespace std;

#ifndef _MSC_VER
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <Windows.h>
#include <locale>
#include <direct.h>
#endif

#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

bool FileUtils::IsFileExist(const string & fileName)
{
	ifstream inFile(fileName);
	return inFile.good();
}

bool FileUtils::IsFileWritable(const string & fileName)
{
	string folder;
	GetFolderOfFile(fileName, folder);
	if (!CreateFolder(folder))
		return false;

	ofstream outFile;
	outFile.open(fileName, ofstream::binary | ofstream::out);
	bool ok = outFile.is_open();
	if (ok) {
		outFile.close();
		RemoveFile(fileName);
	}
	return ok;
}

bool FileUtils::IsFileEmpty(string & filePath)
{
	ifstream pFile(filePath);
	return pFile.peek() == ifstream::traits_type::eof();
}

bool FileUtils::IsFolder(const string & path)
{
	struct stat s;
	if (stat(path.c_str(), &s) != 0) return false;
	return ((s.st_mode & S_IFDIR) != 0);
}

string FileUtils::GetFileExtension(const string & fileName)
{
	string::size_type idx;
	idx = fileName.rfind('.');
	string extension = "";
	if (idx != string::npos)
		extension = fileName.substr(idx + 1);
	transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension;
}

vector<string> FileUtils::GetFilesInFolder(const string& path)
{
#ifdef __linux__
	auto ret = vector<string>();
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.data())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			ret.push_back(ent->d_name);
		}
		closedir(dir);
	}
	return ret;
#else
	vector<string> names;
	string pathAsterix = path + "/*.*";
	wstring search_path = wstring(pathAsterix.begin(), pathAsterix.end());
	WIN32_FIND_DATAW fd;
	HANDLE hFind = ::FindFirstFileW(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(StringUtils::ToNarrow(fd.cFileName));
			}
		} while (::FindNextFileW(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
#endif
}

#ifndef __linux__ // wchar stuff
string StringUtils::ToNarrow(const wchar_t *s, char dfault, const locale& loc)
{
	ostringstream stm;

	while (*s != L'\0') {
		stm << use_facet< ctype<wchar_t> >(loc).narrow(*s++, dfault);
	}
	return stm.str();
}

std::wstring StringUtils::ToWide(const char * s)
{
	size_t len = strlen(s);
	std::wstring wc(len, L'#');
	mbstowcs(&wc[0], s, len);
	return wc;
}
#endif

string FileUtils::GetTempFolder()
{
#ifndef __linux__ 
	wstring tempPath;
	wchar_t wcharPath[MAX_PATH];
	if (GetTempPathW(MAX_PATH, wcharPath))
		tempPath = wcharPath;
	return StringUtils::Replace(StringUtils::ToNarrow(tempPath.c_str()), "\\", "/");
#else
	return "/tmp/";
#endif
}

void FileUtils::RemoveFile(const string & fName)
{
	remove(fName.c_str());
}

size_t FileUtils::GetFileSize(const string & fName)
{
	if (!IsFileExist(fName)) return 0;
	ifstream in(fName, ifstream::ate | ifstream::binary);
	return in.tellg();
}

void FileUtils::GetFolderOfFile(const string & fName, string & folder)
{
	size_t pos = fName.find_last_of("\\/");
	folder = (wstring::npos == pos)
		? ""
		: fName.substr(0, pos);
}

void FileUtils::GetExecutableFolder(string & folder)
{
#ifdef _MSC_VER // windows
	wchar_t ownPth[MAX_PATH];
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileNameW(hModule, ownPth, (sizeof(ownPth)));
	folder = StringUtils::ToNarrow(ownPth);
#else
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	folder = string(result, (count > 0) ? count : 0);
#endif
	GetFolderOfFile(folder, folder);
}

static int myMakedir(const char* path)
{
#ifdef _MSC_VER
	return mkdir(path);
#else
	return mkdir(path, 0733);
#endif
}

bool FileUtils::CreateFolder(const string & _fullPath)
{ // https://stackoverflow.com/questions/10402499/mkdir-c-function
	string fullPath = _fullPath;
	replace(fullPath.begin(), fullPath.end(), '\\', '/');
	if (_fullPath.size() == 0) // edge case
		return false;

	if (fullPath.back() == '/')
		fullPath.pop_back();

	if (_fullPath.size() == 0)
		return false; // edge case

	string current_level = "";
	string level;
	stringstream ss(fullPath);

	// split path using slash as a separator
	while (getline(ss, level, '/')) {
		current_level += level; // append folder to the current level

		// create current level
		if (!IsFolder(current_level + "/") && myMakedir(current_level.c_str()) != 0)
			return false;

		current_level += "/"; // don't forget to append a slash
	}
	return true;
}

void FileUtils::GetFileName(const string & _fullPath, string& fName, bool keepExtension)
{
	fName = _fullPath;
	replace(fName.begin(), fName.end(), '\\', '/');
	const size_t lastSlashIdx = fName.find_last_of('/');
	if (string::npos != lastSlashIdx) {
		fName.erase(0, lastSlashIdx + 1);
	}

	if (!keepExtension) {
		const size_t periodIdx = fName.rfind('.');
		if (string::npos != periodIdx) {
			fName.erase(periodIdx);
		}
	}
}

bool FileUtils::Write(const std::string & fullPath, std::string text)
{	;
	ofstream outFile;
	outFile.open(fullPath, ofstream::out | ofstream::trunc);
	bool ok = outFile.is_open();
	outFile << text;
	outFile.close();
	return ok;
}

bool FileUtils::Copy(const std::string& from, const std::string& to)
{ //https://stackoverflow.com/a/10195497/2378218
	std::ifstream  src(from, std::ios::binary);
	std::ofstream  dst(to, std::ios::binary);
	if (!src.is_open()) return false;
	if (!dst.is_open()) return false;
	dst << src.rdbuf();

	return true;
}

std::string FileUtils::ReadAll(std::string fName, bool* ok)
{
	// stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring/2602258#2602258
	std::ifstream t(fName);
	bool isOpen = t.is_open();
	if (ok != nullptr) *ok = isOpen;
	if (isOpen) {
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		std::string buffer(size, '\0');
		t.seekg(0);
		t.read(&buffer[0], size);
		return buffer;
	}
	else 
		return "";
}

std::string FileUtils::WriteToTempFile(const std::string & text, const std::string _fName)
{
	auto fName = GetTempFolder() + "/" + _fName;
	Write(fName, text);
	return fName;
}

#ifdef _MSC_VER // wchar stuff
void FileUtils::GetExecutableFolder(wstring& folder)
{
	wchar_t ownPth[MAX_PATH];
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileNameW(hModule, ownPth, (sizeof(ownPth)));
	folder = ownPth;
	GetFolderOfFile(folder, folder);
}

void FileUtils::GetFolderOfFile(const wstring & fName, wstring& folder)
{
	size_t pos = fName.find_last_of(L"\\/");
	folder = (wstring::npos == pos)
		? L"./"
		: fName.substr(0, pos);
}

bool FileUtils::IsFileExist(const wstring& fileName)
{
	ifstream inFile(fileName);
	return inFile.good();
}

bool FileUtils::IsFileWritable(const wstring& fileName)
{
	ofstream outFile;
	outFile.open(fileName, ofstream::binary | ofstream::out | ofstream::trunc);
	bool ok = outFile.is_open();
	if (ok) {
		outFile.close();
		RemoveFile(fileName.data());
	}
	return ok;
}

void FileUtils::RemoveFile(const wstring & fName)
{
	DeleteFileW(fName.c_str());
}
#endif

vector<string> StringUtils::Split(const string & str, char delim, bool keepEmptyParts)
{
	return Split(str, string(1, delim), keepEmptyParts);
}

vector<string> StringUtils::Split(const string & str, const string & delim, bool keepEmptyParts)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (keepEmptyParts || !token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

string StringUtils::Trim(const string & str)
{
	return TrimLeft(TrimRight(str));
}

string StringUtils::TrimLeft(const string & str)
{
	auto s = str;
	s.erase(s.begin(), find_if(s.begin(), s.end(),
		not1(ptr_fun<int, int>(isspace))));
	return s;
}

string StringUtils::TrimRight(const string & str)
{
	auto s = str;
	s.erase(std::find(s.begin(), s.end(), '\0'), s.end());
	s.erase(find_if(s.rbegin(), s.rend(),
		not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

std::string StringUtils::Replace(const std::string & str, const std::string & what, const std::string & with)
{
	string ret = str;
	size_t index = 0;
	while (true) {
		index = ret.find(what, index);
		if (index == std::string::npos) break;
		ret.replace(index, what.length(), with);
		index += what.length();
	}
	return ret;
}

std::string StringUtils::Join(const std::vector<std::string>& strings, char delim, bool keepEmptyParts)
{
	string ret = "";
	for (size_t i = 0; i < strings.size(); i++) {
		const auto& s = strings[i];
		if (!keepEmptyParts && s.empty()) {
			continue;
		}
		ret += s;
		if (i + 1 < strings.size())
			ret += delim;
	}
	return ret;
}

Stopwatch::Stopwatch(std::string functionName) :
	msg(functionName) {
	start = std::chrono::steady_clock::now();
}

Stopwatch::~Stopwatch() {
	end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds = end - start;
	double seconds = elapsedSeconds.count();
	cout << msg << " took " << seconds << "[sec]" << endl;
}
