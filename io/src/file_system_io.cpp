/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <cassert>
#include <string>
#include <vector>
#include <errno.h>
#include "io/_private/_filesystem_api.h"
#include "io/_private/_file_time.h"
#include "io/file_system_io.h"

using namespace pandora::io;

// -- path metadata - absolute/relative -- -------------------------------------

#ifdef _WINDOWS // advanced verification for absolute paths (Windows)
  template <typename _CharType, _CharType _MediaSep, _CharType _LevelSep1, _CharType _LevelSep2>
  inline bool _isPathAbsolute(const _CharType* path) noexcept {
    if (path != nullptr) {
      if (path[0] == _LevelSep1 || path[0] == _LevelSep2)
        return true;
      while (*path != static_cast<_CharType>(0) && *path != _MediaSep)
        ++path;
      return (*path != static_cast<_CharType>(0) && (path[1] == _LevelSep1 || path[1] == _LevelSep2));
    }
    return false;
  }
#endif

// - get absolute/relative path type -
FileSystemPathType pandora::io::getFileSystemPathType(const char* path) noexcept {
# ifdef _WINDOWS
    return _isPathAbsolute<char,':','/','\\'>(path) ? FileSystemPathType::absolute : FileSystemPathType::relative;
# else
    return (path != nullptr && (*path == '/' || *path == '\\')) ? FileSystemPathType::absolute : FileSystemPathType::relative;
# endif
}
#ifdef _WINDOWS
  FileSystemPathType pandora::io::getFileSystemPathType(const wchar_t* path) noexcept {
    return _isPathAbsolute<wchar_t,L':',L'/',L'\\'>(path) ? FileSystemPathType::absolute : FileSystemPathType::relative;
  }
#endif

// get separator to append to a directory path
static inline char _getPathSuffixSeparator(const char* path) noexcept {
# ifdef _WINDOWS
    return (getFileSystemPathType(path) == FileSystemPathType::absolute) ? '\\' : '/';
# else
    return '/';
# endif
}
#ifdef _WINDOWS
  static inline wchar_t _getPathSuffixSeparator(const wchar_t* path) noexcept {
    return (getFileSystemPathType(path) == FileSystemPathType::absolute) ? L'\\' : L'/';
  }
#endif


// -- path metadata - entry type -- --------------------------------------------

#ifdef _WINDOWS // read entry type from attributes - windows native
  static inline FileSystemEntryType _readFileSystemEntryType_native(DWORD attributes) noexcept {
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
      return FileSystemEntryType::directory;
    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
      return FileSystemEntryType::symlink;
    return FileSystemEntryType::file;
  }
#else // read entry type from attributes - linux/unix native
  static inline FileSystemEntryType _readFileSystemEntryType_native(const struct stat& attributes) noexcept {
    if (S_ISDIR(attributes.st_mode) != 0)
      return FileSystemEntryType::directory;
    if (S_ISREG(attributes.st_mode) != 0)
      return FileSystemEntryType::file;
    if (S_ISLNK(attributes.st_mode) != 0)
      return FileSystemEntryType::symlink;
    if (S_ISSOCK(attributes.st_mode) != 0)
      return FileSystemEntryType::socket;
    return FileSystemEntryType::unknown;
  }
#endif
#ifndef _USE_NATIVE_FILESYSTEM
  // read entry type from attributes - C++17
  static inline FileSystemEntryType _readFileSystemEntryType_cpp17(const std::filesystem::directory_entry& dirEntry, std::error_code& errorCode) noexcept {
    if (dirEntry.is_directory(errorCode))
      return FileSystemEntryType::directory;
    if (dirEntry.is_regular_file(errorCode))
      return FileSystemEntryType::file;
    if (dirEntry.is_symlink(errorCode))
      return FileSystemEntryType::symlink;
    if (dirEntry.is_socket(errorCode))
      return FileSystemEntryType::socket;
    return FileSystemEntryType::unknown;
  }
  // read entry type from attributes - C++17 with native fallback
  static FileSystemEntryType _readFileSystemEntryType_fallback(const std::filesystem::directory_entry& dirEntry, std::error_code& errorCode) noexcept {
    FileSystemEntryType entryType = _readFileSystemEntryType_cpp17(dirEntry, errorCode);
    if (entryType == FileSystemEntryType::unknown) {
#     ifdef _WINDOWS
        auto entryPath = dirEntry.path().wstring();
        return _readFileSystemEntryType_native(GetFileAttributesW(entryPath.c_str()));
#     else
        struct stat pathInfo;
        auto entryPath = dirEntry.path().string();
        return (stat(entryPath.c_str(), &pathInfo) == 0) ? _readFileSystemEntryType_native(pathInfo) : FileSystemEntryType::unknown;
#     endif
    }
    return entryType;
  }
#endif

// - get entry type from path -
FileSystemEntryType pandora::io::getFileSystemEntryType(const char* path) noexcept {
  if (path == nullptr || *path == 0)
    return FileSystemEntryType::unknown;

# ifndef _USE_NATIVE_FILESYSTEM
    std::error_code errorCode; // to disable exceptions
    return _readFileSystemEntryType_fallback(std::filesystem::directory_entry(std::filesystem::path(path), errorCode), errorCode);
# else
#   ifdef _WINDOWS
      return (PathFileExistsA(path) == TRUE) ? _readFileSystemEntryType_native(GetFileAttributesA(path)) : FileSystemEntryType::unknown;
#   else
      struct stat pathInfo;
      return (stat(path, &pathInfo) == 0) ? _readFileSystemEntryType_native(pathInfo) : FileSystemEntryType::unknown;
#   endif
# endif
}
#ifdef _WINDOWS
  FileSystemEntryType pandora::io::getFileSystemEntryType(const wchar_t* path) noexcept {
    if (path == nullptr || *path == 0)
      return FileSystemEntryType::unknown;

#   ifndef _USE_NATIVE_FILESYSTEM
      std::error_code errorCode; // to disable exceptions
      return _readFileSystemEntryType_fallback(std::filesystem::directory_entry(std::filesystem::path(path), errorCode), errorCode);
#   else
      return (PathFileExistsW(path) == TRUE) ? _readFileSystemEntryType_native(GetFileAttributesW(path)) : FileSystemEntryType::unknown;
#   endif
  }
#endif


// -- path metadata - hidden files -- ------------------------------------------

#ifdef _WINDOWS // check if an entry is hidden - windows native
  static inline bool _isHiddenFileSystemEntry_native(DWORD attributes) noexcept {
    return ((attributes & FILE_ATTRIBUTE_HIDDEN) != 0);
  }
#else // check if an entry is hidden - linux/unix native
  static inline bool _isHiddenFileSystemEntry_native(const char* fileName) noexcept {
    return (fileName != nullptr && fileName[0] == '.' 
         && fileName[1] != 0 && fileName[1] != '/' && (fileName[1] != '.' || (fileName[2] != 0 && fileName[2] != '/')) );
  }
#endif

// - check if an entry is hidden -
bool pandora::io::isFileSystemEntryHidden(const char* path) noexcept {
# ifdef _WINDOWS
    return (path != nullptr && _isHiddenFileSystemEntry_native(GetFileAttributesA(path)));
# else
    size_t last = (path != nullptr) ? std::string(path).find_last_of('/') : std::string::npos;
    return (last == std::string::npos) ? _isHiddenFileSystemEntry_native(path) : _isHiddenFileSystemEntry_native(&path[last + 1u]);
# endif
}
#ifdef _WINDOWS
  bool pandora::io::isFileSystemEntryHidden(const wchar_t* path) noexcept {
    return (path != nullptr && _isHiddenFileSystemEntry_native(GetFileAttributesW(path)));
  }
#endif


// -- path metadata - existence & access mode -- -------------------------------

// get available access mode for a path (doesn't guarantee the file exists!)
#ifndef _WINDOWS
  static inline FileSystemAccessMode _getFileSystemAccessMode(const char* path) noexcept {
    FileSystemAccessMode accessMode = (access(path, static_cast<int32_t>(FileSystemAccessMode::read)) == 0)
                                    ? FileSystemAccessMode::read : FileSystemAccessMode::existence;
    if (access(path, static_cast<int32_t>(FileSystemAccessMode::write)) == 0)
      accessMode = static_cast<FileSystemAccessMode>((int32_t)accessMode | (int32_t)FileSystemAccessMode::write);
    return accessMode;
  }
#else
  static inline FileSystemAccessMode _getFileSystemAccessMode(const char* path) noexcept {
    FileSystemAccessMode accessMode = (_access(path, static_cast<int>(FileSystemAccessMode::read)) == 0)
                                    ? FileSystemAccessMode::read : FileSystemAccessMode::existence;
    if (_access(path, static_cast<int>(FileSystemAccessMode::write)) == 0)
      accessMode = static_cast<FileSystemAccessMode>((int32_t)accessMode | (int32_t)FileSystemAccessMode::write);
    return accessMode;
  }
  static inline FileSystemAccessMode _getFileSystemAccessMode(const wchar_t* path) noexcept {
    FileSystemAccessMode accessMode = (_waccess(path, static_cast<int32_t>(FileSystemAccessMode::read)) == 0)
                                    ? FileSystemAccessMode::read : FileSystemAccessMode::existence;
    if (_waccess(path, static_cast<int32_t>(FileSystemAccessMode::write)) == 0)
      accessMode = static_cast<FileSystemAccessMode>((int32_t)accessMode | (int32_t)FileSystemAccessMode::write);
    return accessMode;
  }
#endif

// - check availability of an access mode for an entry -
#ifndef _WINDOWS
  bool pandora::io::verifyFileSystemAccessMode(const char* path, FileSystemAccessMode mode) noexcept {
    return (path != nullptr && access(path, static_cast<int32_t>(mode)) == 0);
  }
#else
  bool pandora::io::verifyFileSystemAccessMode(const char* path, FileSystemAccessMode mode) noexcept {
    return (path != nullptr && _access(path, static_cast<int32_t>(mode)) == 0);
  }
  bool pandora::io::verifyFileSystemAccessMode(const wchar_t* path, FileSystemAccessMode mode) noexcept {
    return (path != nullptr && _waccess(path, static_cast<int32_t>(mode)) == 0);
  }
#endif


// -- path metadata - path segments -- -----------------------------------------

static inline size_t _findPathDelimiter(const std::string& path, size_t offset) noexcept {
  return path.find_first_of("\\/", offset);
}
#ifdef _WINDOWS
  static inline size_t _findPathDelimiter(const std::wstring& path, size_t offset) noexcept {
    return path.find_first_of(L"\\/", offset);
  }
#endif

// split a path into segments (implementation)
template <typename _CharType>
inline std::vector<std::basic_string<_CharType> > _getFileSystemPathSegments(const std::basic_string<_CharType>& path) noexcept {
  std::vector<std::basic_string<_CharType> > segments;
  size_t current = 0u;
  size_t last = 0u;
  
  if (!path.empty()) {
    current = _findPathDelimiter(path, 0u);
    while (current != std::basic_string<_CharType>::npos) {
      if (current != last)
        segments.emplace_back(path.substr(last, current - last));
      last = current + 1;
      current = _findPathDelimiter(path, last);
    }
    if (last < path.size())
      segments.emplace_back(path.substr(last));
  }
  return segments;
}

// - split a path into segments -
std::vector<std::string> pandora::io::getFileSystemPathSegments(const std::string& path) noexcept { return _getFileSystemPathSegments<char>(path); }
#ifdef _WINDOWS
  std::vector<std::wstring> pandora::io::getFileSystemPathSegments(const std::wstring& path) noexcept { return _getFileSystemPathSegments<wchar_t>(path); }
#endif


// -- entry creation/removal -- -------------------------------------------------

// - create/open file entry -
pandora::io::FileHandle pandora::io::openFileEntry(const char* path, const char* mode) noexcept {
  return (path != nullptr) ? pandora::io::FileHandle(fopen(path, mode)) : pandora::io::FileHandle{};
}
#ifdef _WINDOWS
  pandora::io::FileHandle pandora::io::openFileEntry(const wchar_t* path, const wchar_t* mode) noexcept {
    FILE* fileAccess = nullptr;
    if (path != nullptr && _wfopen_s(&fileAccess, path, mode) == 0)
      return pandora::io::FileHandle(fileAccess);
    errno = 0;
    return pandora::io::FileHandle{};
  }
#endif

// - remove existing file entry -
int pandora::io::removeFileEntry(const std::string& path) noexcept {
  if (remove(path.c_str()) == 0)
    return 0;
  int errorCode = (errno != 0) ? errno : -1;
  errno = 0;
  return errorCode;
}
#ifdef _WINDOWS
  int pandora::io::removeFileEntry(const std::wstring& path) noexcept {
    if (_wremove(path.c_str()) == 0)
      return 0;
    int errorCode = (errno != 0) ? errno : -1;
    errno = 0;
    return errorCode;
  }
#endif

// - create directory entry -
int pandora::io::createDirectory(const std::string& path) noexcept {
# ifdef _WINDOWS
    if (_mkdir(path.c_str()) == 0)
      return 0;
# else
    if (mkdir(path.c_str(), (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) == 0)
      return 0;
# endif
  int errorCode = (errno != 0) ? errno : -1;
  errno = 0;
  return errorCode;
}
#ifdef _WINDOWS
  int pandora::io::createDirectory(const std::wstring& path) noexcept {
    if (_wmkdir(path.c_str()) == 0)
      return 0;
    int errorCode = (errno != 0) ? errno : -1;
    errno = 0;
    return errorCode;
  }
#endif

// - remove existing directory entry -
int pandora::io::removeDirectory(const std::string& path) noexcept {
# ifdef _WINDOWS
    if (_rmdir(path.c_str()) == 0)
      return 0;
# else
    if (rmdir(path.c_str()) == 0)
      return 0;
# endif
  int errorCode = (errno != 0) ? errno : -1;
  errno = 0;
  return errorCode;
}
#ifdef _WINDOWS
  int pandora::io::removeDirectory(const std::wstring& path) noexcept {
    if (_wrmdir(path.c_str()) == 0)
      return 0;
    int errorCode = (errno != 0) ? errno : -1;
    errno = 0;
    return errorCode;
  }
#endif


// -- path metadata - directory / file metadata -- -----------------------------

#ifdef _USE_NATIVE_FILESYSTEM
# ifdef _WINDOWS
    static inline HANDLE _findFirstFileInfo_native(const char* path, WIN32_FIND_DATAA& fileInfo) noexcept { return FindFirstFileA(path, &fileInfo); }
    static inline HANDLE _findFirstFileInfo_native(const wchar_t* path, WIN32_FIND_DATAW& fileInfo) noexcept { return FindFirstFileW(path, &fileInfo); }
    static inline BOOL _findNextFileInfo_native(HANDLE& fileHandle, WIN32_FIND_DATAA& fileInfo) noexcept { return FindNextFileA(fileHandle, &fileInfo); }
    static inline BOOL _findNextFileInfo_native(HANDLE& fileHandle, WIN32_FIND_DATAW& fileInfo) noexcept { return FindNextFileW(fileHandle, &fileInfo); }
    
    // fill metadata structure for a file system entry - windows native
    template <typename _CharType, typename _FileInfo, bool _WideChar>
    static inline FileSystemEntryMetadata<_WideChar> _fillFileSystemEntryMetadata_native(const _CharType* path, _FileInfo& fileInfo) noexcept {
      return FileSystemEntryMetadata<_WideChar>{
        std::basic_string<_CharType>(fileInfo.cFileName),
        _toWriteTime(fileInfo.ftLastWriteTime),
        (static_cast<uintmax_t>(fileInfo.nFileSizeHigh) << sizeof(fileInfo.nFileSizeLow) * 8) | static_cast<uintmax_t>(fileInfo.nFileSizeLow),
        _readFileSystemEntryType_native(fileInfo.dwFileAttributes),
        _getFileSystemAccessMode(path),
        _isHiddenFileSystemEntry_native(fileInfo.dwFileAttributes)
      };
    }
    // read file system entry metadata - windows native
    template <typename _CharType, typename _FileInfo, bool _WideChar>
    static inline bool _readFileSystemEntryMetadata_native(const _CharType* path, FileSystemEntryMetadata<_WideChar>& outMetadata) noexcept {
      _FileInfo fileInfo;
      HANDLE fileHandle = _findFirstFileInfo_native(path, fileInfo);
      SetLastError(0);
      if (fileHandle == INVALID_HANDLE_VALUE)
        return false;

      outMetadata = _fillFileSystemEntryMetadata_native<_CharType,_FileInfo,_WideChar>(path, fileInfo);
      DWORD error = GetLastError();
      FindClose(fileHandle);
      SetLastError(0);
      return (error == 0 || error == ERROR_NO_MORE_FILES);
    }
# else // read file system entry metadata - linux/unix native
    static inline bool _readFileSystemEntryMetadata_native(const std::string& path, FileSystemEntryMetadata& outMetadata) noexcept {
      if (!verifyFileSystemAccessMode(path.c_str(), FileSystemAccessMode::existence))
        return false;
      size_t lastSeparator = path.find_last_of("/\\");

      struct stat fileMetadata;
      stat(path.c_str(), &fileMetadata);
      outMetadata = FileSystemEntryMetadata{
        std::string((lastSeparator != std::string::npos) ? path.substr(lastSeparator + 1u) : path),
        fileMetadata.st_mtime,
        static_cast<uintmax_t>(fileMetadata.st_size),
        _readFileSystemEntryType_native(fileMetadata),
        _getFileSystemAccessMode(path.c_str()),
        false
      };
      outMetadata.isHidden = _isHiddenFileSystemEntry_native(outMetadata.name.c_str());
      return true;
    }
# endif
#else // C++17
  template <typename _CharType>
  inline std::basic_string<_CharType> _extractFileName_cpp17(const std::filesystem::directory_entry& entry) noexcept;
  template <> inline std::basic_string<char> _extractFileName_cpp17<char>(const std::filesystem::directory_entry& entry) noexcept { return entry.path().filename().string(); }
  template <> inline std::basic_string<wchar_t> _extractFileName_cpp17<wchar_t>(const std::filesystem::directory_entry& entry) noexcept { return entry.path().filename().wstring(); }
    
  // read file system entry metadata - C++17
  template <typename _CharType, typename _EntryMetadata>
  static inline bool _readFileSystemEntryMetadata_cpp17(const std::basic_string<_CharType>& path, _EntryMetadata& outMetadata) noexcept {
    std::error_code errorCode; // to disable exceptions
    std::filesystem::directory_entry entry(path, errorCode);
    if (errorCode || !entry.exists())
      return false;

    outMetadata = _EntryMetadata{
      _extractFileName_cpp17<_CharType>(entry),
      _toWriteTime(entry),
      entry.file_size(errorCode),
      _readFileSystemEntryType_fallback(entry, errorCode),
      _getFileSystemAccessMode(path.c_str()),
#     ifdef _WINDOWS
        isFileSystemEntryHidden(path.c_str())
#     else
        false
#     endif
    };
#   ifndef _WINDOWS
      outMetadata.isHidden = _isHiddenFileSystemEntry_native(outMetadata.name.c_str());
#   endif
    return true;
  }
#endif

// - read file system entry metadata -
#ifndef _WINDOWS
  bool pandora::io::readFileSystemEntryMetadata(const std::string& path, FileSystemEntryMetadata& out) noexcept {
    auto noSepPath = path;
    while (!noSepPath.empty() && (noSepPath.back() == '/' || noSepPath.back() == '\\'))
      noSepPath.pop_back();

#   ifndef _USE_NATIVE_FILESYSTEM
      return _readFileSystemEntryMetadata_cpp17(noSepPath, out);
#   else
      return _readFileSystemEntryMetadata_native(noSepPath, out);
#   endif
  }
#else
  bool pandora::io::readFileSystemEntryMetadata(const std::string& path, FileSystemEntryMetadata<false>& out) noexcept {
    auto noSepPath = path;
    while (!noSepPath.empty() && (noSepPath.back() == '/' || noSepPath.back() == '\\'))
      noSepPath.pop_back();

#   ifndef _USE_NATIVE_FILESYSTEM
      return _readFileSystemEntryMetadata_cpp17<char,FileSystemEntryMetadata<false> >(noSepPath, out);
#   else
      return _readFileSystemEntryMetadata_native<char,WIN32_FIND_DATAA,false>(noSepPath.c_str(), out);
#   endif
  }
  bool pandora::io::readFileSystemEntryMetadata(const std::wstring& path, FileSystemEntryMetadata<true>& out) noexcept {
    auto noSepPath = path;
    while (!noSepPath.empty() && (noSepPath.back() == L'/' || noSepPath.back() == L'\\'))
      noSepPath.pop_back();

#   ifndef _USE_NATIVE_FILESYSTEM
      return _readFileSystemEntryMetadata_cpp17<wchar_t,FileSystemEntryMetadata<true> >(noSepPath, out);
#   else
      return _readFileSystemEntryMetadata_native<wchar_t,WIN32_FIND_DATAW,true>(noSepPath.c_str(), out);
#   endif
  }
#endif


// -- list directory content & metadata -- -------------------------------------

#ifdef _USE_NATIVE_FILESYSTEM
# ifdef _WINDOWS 
    // list file system entries in a directory - windows native
    template <typename _CharType, typename _FileInfo, bool _WideChar>
    static inline bool _listFileSystemEntries_native(std::basic_string<_CharType>& directoryPathWithSeparator, std::vector<FileSystemEntry<_WideChar> >& outList) noexcept {
      _FileInfo fileInfo;
      directoryPathWithSeparator.push_back(static_cast<_CharType>('*')); // query subcontent
      HANDLE fileHandle = _findFirstFileInfo_native(directoryPathWithSeparator.c_str(), fileInfo);
      directoryPathWithSeparator.pop_back();
      SetLastError(0);
      if (fileHandle == INVALID_HANDLE_VALUE)
        return false;

      do {
        outList.push_back( FileSystemEntry<_WideChar>(std::basic_string<_CharType>(fileInfo.cFileName), _readFileSystemEntryType_native(fileInfo.dwFileAttributes)) );
      } while (_findNextFileInfo_native(fileHandle, fileInfo) != 0);

      DWORD error = GetLastError();
      FindClose(fileHandle);
      SetLastError(0);
      return (error == 0 || error == ERROR_NO_MORE_FILES);
    }
    // read metadata of a list of file system entries - windows native
    template <typename _CharType, typename _FileInfo, bool _WideChar>
    static inline bool _readFileSystemListMetadata_native(std::basic_string<_CharType>& directoryPathWithSeparator, std::vector<FileSystemEntryMetadata<_WideChar> >& outMetadataList) noexcept {
      _FileInfo fileInfo;
      directoryPathWithSeparator.push_back(static_cast<_CharType>('*')); // query subcontent
      HANDLE fileHandle = _findFirstFileInfo_native(directoryPathWithSeparator.c_str(), fileInfo);
      directoryPathWithSeparator.pop_back();
      SetLastError(0);
      if (fileHandle == INVALID_HANDLE_VALUE)
        return false;

      do {
        auto fullPath = directoryPathWithSeparator + fileInfo.cFileName;
        outMetadataList.emplace_back(_fillFileSystemEntryMetadata_native<_CharType,_FileInfo,_WideChar>(fullPath.c_str(), fileInfo));
      } while (_findNextFileInfo_native(fileHandle, fileInfo) != 0);

      DWORD error = GetLastError();
      FindClose(fileHandle);
      SetLastError(0);
      return (error == 0 || error == ERROR_NO_MORE_FILES);
    }
# else 
    // list file system entries in a directory - linux/unix native
    static inline bool _listFileSystemEntries_native(const std::string& path, std::vector<FileSystemEntry>& outList) noexcept {
      DIR* access = opendir(path.c_str());
      if (access == nullptr)
        return false;

      struct dirent* fileInfo = nullptr;
      while ((fileInfo = readdir(access)) != nullptr)
        outList.emplace_back(fileInfo->d_name);
      closedir(access);
      return true;
    }
    // read metadata of a list of file system entries - linux/unix native
    static inline bool _readFileSystemListMetadata_native(const std::string& directoryPathWithSeparator, std::vector<FileSystemEntryMetadata>& outMetadataList) noexcept {
      DIR* access = opendir(directoryPathWithSeparator.c_str());
      if (access == nullptr)
        return false;

      struct stat fileMetadata;
      struct dirent* fileInfo = nullptr;
      while ((fileInfo = readdir(access)) != nullptr) {
        auto fullPath = directoryPathWithSeparator + fileInfo->d_name;
        stat(fullPath.c_str(), &fileMetadata);

        outMetadataList.emplace_back(FileSystemEntryMetadata{
            std::string(fileInfo->d_name),
            fileMetadata.st_mtime,
            static_cast<uintmax_t>(fileMetadata.st_size),
            _readFileSystemEntryType_native(fileMetadata),
            _getFileSystemAccessMode(fullPath.c_str()),
            _isHiddenFileSystemEntry_native(fileInfo->d_name)
        });
      }
      closedir(access);
      return true;
    }
# endif
#else 
  // list file system entries in a directory - C++17
  template <typename _CharType, typename _Entry>
  static inline bool _listFileSystemEntries_cpp17(const std::basic_string<_CharType>& path, std::vector<_Entry>& outList) noexcept {
    std::error_code errorCode; // to disable exceptions
    for (auto& entry : std::filesystem::directory_iterator(path, errorCode)) {
      if (errorCode || !entry.exists())
        continue;
      outList.emplace_back(_extractFileName_cpp17<_CharType>(entry), _readFileSystemEntryType_fallback(entry, errorCode));
    }
    return (!errorCode);
  }
  // read metadata of a list of file system entries - C++17
  template <typename _CharType, typename _EntryMetadata>
  static inline bool _readFileSystemListMetadata_cpp17(const std::basic_string<_CharType>& directoryPathWithSeparator, 
                                                       std::vector<_EntryMetadata>& outMetadataList) noexcept {
    std::error_code errorCode; // to disable exceptions
    for (auto& entry : std::filesystem::directory_iterator(directoryPathWithSeparator, errorCode)) {
      if (errorCode || !entry.exists())
        continue;
      
      outMetadataList.emplace_back(_EntryMetadata{
        _extractFileName_cpp17<_CharType>(entry),
        _toWriteTime(entry),
        entry.file_size(errorCode),
        _readFileSystemEntryType_fallback(entry, errorCode),
        FileSystemAccessMode::existence,
        false
      });
    
      auto fullPath = directoryPathWithSeparator + outMetadataList.back().name;
      outMetadataList.back().accessMode = _getFileSystemAccessMode(fullPath.c_str());
#     ifdef _WINDOWS
      outMetadataList.back().isHidden = isFileSystemEntryHidden(fullPath.c_str());
#     else
        outMetadataList.back().isHidden = _isHiddenFileSystemEntry_native(outMetadataList.back().name.c_str());
#     endif
    }
    return (!errorCode);
  }
#endif

// - read metadata of a list of file system entries -
#ifndef _WINDOWS
  bool pandora::io::listFilesInDirectory(const std::string& path, std::vector<FileSystemEntryMetadata>& outFiles) noexcept {
    auto directoryPathWithSeparator = (!path.empty() && path.back() != '/' && path.back() != '\\') ? path +  _getPathSuffixSeparator(path.c_str()) : path;
    outFiles.clear();
#   ifndef _USE_NATIVE_FILESYSTEM
      return _readFileSystemListMetadata_cpp17<char,FileSystemEntryMetadata>(directoryPathWithSeparator, outFiles);
#   else
      return _readFileSystemListMetadata_native(directoryPathWithSeparator, outFiles);
#   endif
  }
#else
  bool pandora::io::listFilesInDirectory(const std::string& path, std::vector<FileSystemEntryMetadata<false> >& outFiles) noexcept {
    auto directoryPathWithSeparator = (!path.empty() && path.back() != '/' && path.back() != '\\') ? path +  _getPathSuffixSeparator(path.c_str()) : path;
    outFiles.clear();
#   ifndef _USE_NATIVE_FILESYSTEM
      return _readFileSystemListMetadata_cpp17<char,FileSystemEntryMetadata<false> >(directoryPathWithSeparator, outFiles);
#   else
      return _readFileSystemListMetadata_native<char,WIN32_FIND_DATAA,false>(directoryPathWithSeparator, outFiles);
#   endif
  }
  bool pandora::io::listFilesInDirectory(const std::wstring& path, std::vector<FileSystemEntryMetadata<true> >& outFiles) noexcept {
    auto directoryPathWithSeparator = (!path.empty() && path.back() != L'/' && path.back() != L'\\') ? path +  _getPathSuffixSeparator(path.c_str()) : path;
    outFiles.clear();
#   ifndef _USE_NATIVE_FILESYSTEM
      return _readFileSystemListMetadata_cpp17<wchar_t,FileSystemEntryMetadata<true> >(directoryPathWithSeparator, outFiles);
#   else
      return _readFileSystemListMetadata_native<wchar_t,WIN32_FIND_DATAW,true>(directoryPathWithSeparator, outFiles);
#   endif
  }
#endif

// - list file system entries in a directory -
#ifndef _WINDOWS
  bool pandora::io::listFilesInDirectory(const std::string& path, std::vector<FileSystemEntry>& outFiles) noexcept {
    auto directoryPathWithSeparator = (!path.empty() && path.back() != '/' && path.back() != '\\') ? path +  _getPathSuffixSeparator(path.c_str()) : path;
    outFiles.clear();
#   ifndef _USE_NATIVE_FILESYSTEM
      return _listFileSystemEntries_cpp17<char,FileSystemEntry>(directoryPathWithSeparator, outFiles);
#   else
      return _listFileSystemEntries_native(directoryPathWithSeparator, outFiles);
#   endif
  }
#else
  bool pandora::io::listFilesInDirectory(const std::string& path, std::vector<FileSystemEntry<false> >& outFiles) noexcept {
    auto directoryPathWithSeparator = (!path.empty() && path.back() != '/' && path.back() != '\\') ? path +  _getPathSuffixSeparator(path.c_str()) : path;
    outFiles.clear();
#   ifndef _USE_NATIVE_FILESYSTEM
      return _listFileSystemEntries_cpp17<char,FileSystemEntry<false> >(directoryPathWithSeparator, outFiles);
#   else
      return _listFileSystemEntries_native<char,WIN32_FIND_DATAA,false>(directoryPathWithSeparator, outFiles);
#   endif
  }
  bool pandora::io::listFilesInDirectory(const std::wstring& path, std::vector<FileSystemEntry<true> >& outFiles) noexcept {
    auto directoryPathWithSeparator = (!path.empty() && path.back() != L'/' && path.back() != L'\\') ? path +  _getPathSuffixSeparator(path.c_str()) : path;
    outFiles.clear();
#   ifndef _USE_NATIVE_FILESYSTEM
      return _listFileSystemEntries_cpp17<wchar_t,FileSystemEntry<true> >(directoryPathWithSeparator, outFiles);
#   else
      return _listFileSystemEntries_native<wchar_t,WIN32_FIND_DATAW,true>(directoryPathWithSeparator, outFiles);
#   endif
  }
#endif
