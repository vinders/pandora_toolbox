/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <errno.h>
#include <type_traits>
#include "./file_handle.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

namespace pandora { 
  namespace io {
    /// @brief Type of path
    enum class FileSystemPathType: uint32_t {
      absolute = 0,
      relative = 1
    };
    /// @brief Type of entry targeted by a path
    enum class FileSystemEntryType: uint32_t {
      unknown = 0,
      directory = 1,
      file = 2,
      symlink = 3,
      socket = 4
    };
    /// @brief Directory / file access mode to verify
    enum class FileSystemAccessMode: int32_t {
      existence = 0,
      read      = 2,
      write     = 4,
      readWrite = 6
    };
    
    /// @brief Directory / file metadata
#   ifdef _WINDOWS
    template <bool _WideChar = true>
#   endif
    struct FileSystemEntryMetadata {
#     ifdef _WINDOWS
        using Path = typename std::conditional<_WideChar, std::wstring, std::string>::type;
#     else
        using Path = std::string;
#     endif
      Path name;
      time_t lastUpdateTime{ 0 };
      uintmax_t size = 0;
      FileSystemEntryType type = FileSystemEntryType::unknown;
      FileSystemAccessMode accessMode = FileSystemAccessMode::existence;
      bool isHidden = false;
    };
#   ifdef _WINDOWS
      template <bool> class FileSystemEntry;
#   else
      class FileSystemEntry;
#   endif
    
    
    // -- FILE SYSTEM TOOLBOX --------------------------------------------------

    // -- path metadata --
    
    /// @brief Get type of any path
    FileSystemPathType getFileSystemPathType(const char* path) noexcept;
    /// @brief Get type of entry targeted by any path
    FileSystemEntryType getFileSystemEntryType(const char* path);
    /// @brief Verify if any directory / file is hidden
    bool isFileSystemEntryHidden(const char* path) noexcept;
    /// @brief Verify if any path is existing / readable / writable / all
    bool verifyFileSystemAccessMode(const char* path, FileSystemAccessMode mode) noexcept;
    
    /// @brief Split any path (on '/' or '\') into a vector of segments
    std::vector<std::string> getFileSystemPathSegments(const std::string& path);
#   ifndef _WINDOWS
      /// @brief Read metadata of a file / directory
      bool readFileSystemEntryMetadata(const std::string& path, FileSystemEntryMetadata& out);
#   else
      /// @brief Get type of any path
      FileSystemPathType getFileSystemPathType(const wchar_t* path) noexcept;
      /// @brief Get type of entry targeted by any path
      FileSystemEntryType getFileSystemEntryType(const wchar_t* path);
      /// @brief Verify if any directory / file is hidden
      bool isFileSystemEntryHidden(const wchar_t* path) noexcept;
      /// @brief Verify if any path is existing / readable / writable / all
      bool verifyFileSystemAccessMode(const wchar_t* path, FileSystemAccessMode mode) noexcept;
      
      /// @brief Split any path (on '/' or '\') into a vector of segments
      std::vector<std::wstring> getFileSystemPathSegments(const std::wstring& path);
      /// @brief Read metadata of a file / directory
      bool readFileSystemEntryMetadata(const std::string& path, FileSystemEntryMetadata<false>& out);
      /// @brief Read metadata of a file / directory
      bool readFileSystemEntryMetadata(const std::wstring& path, FileSystemEntryMetadata<true>& out);
#   endif

    // -- entry creation/removal --
    
    /// @brief Open buffered file access (r/w/a/r+/w+/a+)
    /// @warning Check result.isOpen() to verify success
    pandora::io::FileHandle openFileEntry(const char* path, const char* mode) noexcept;
    /// @brief Create a file (has no effect if the file exists)
    /// @returns 0 on success, or -1 on failure
    inline int createFileEntry(const std::string& path) noexcept { return (openFileEntry(path.c_str(), "ab").isOpen()) ? 0 : -1; }
    /// @brief Delete an existing file
    /// @returns 0 on success, or the value of errno on failure
    int removeFileEntry(const std::string& path) noexcept;
    /// @brief Create a new directory
    /// @returns 0 on success, or the value of errno on failure
    int createDirectory(const std::string& path) noexcept;
    /// @brief Delete an existing directory
    /// @returns 0 on success, or the value of errno on failure
    int removeDirectory(const std::string& path) noexcept;
#   ifdef _WINDOWS
      /// @brief Open buffered file access (r/w/a/r+/w+/a+)
      /// @warning Check result.isOpen() to verify success
      pandora::io::FileHandle openFileEntry(const wchar_t* path, const wchar_t* mode) noexcept;
      /// @brief Create a file (has no effect if the file exists)
      /// @returns 0 on success, or -1 on failure
      inline int createFileEntry(const std::wstring& path) noexcept { return (openFileEntry(path.c_str(), L"ab").isOpen()) ? 0 : -1; }
      /// @brief Delete an existing file
      /// @returns 0 on success, or the value of errno on failure
      int removeFileEntry(const std::wstring& path) noexcept;
      /// @brief Create a new directory
      /// @returns 0 on success, or the value of errno on failure
      int createDirectory(const std::wstring& path) noexcept;
      /// @brief Delete an existing directory
      /// @returns 0 on success, or the value of errno on failure
      int removeDirectory(const std::wstring& path) noexcept;
#   endif

    // -- list directory content --
    
#   ifndef _WINDOWS
      /// @brief List files in directory
      bool listFilesInDirectory(const std::string& path, std::vector<FileSystemEntry>& outFiles);
      /// @brief List files in directory with their metadata
      bool listFilesInDirectory(const std::string& path, std::vector<FileSystemEntryMetadata>& outFiles);
#   else
      /// @brief List files in directory
      bool listFilesInDirectory(const std::string& path, std::vector<FileSystemEntry<false> >& outFiles);
      /// @brief List files in directory
      bool listFilesInDirectory(const std::wstring& path, std::vector<FileSystemEntry<true> >& outFiles);
      /// @brief List files in directory with their metadata
      bool listFilesInDirectory(const std::string& path, std::vector<FileSystemEntryMetadata<false> >& outFiles);
      /// @brief List files in directory with their metadata
      bool listFilesInDirectory(const std::wstring& path, std::vector<FileSystemEntryMetadata<true> >& outFiles);
#   endif
    
    
    // -- FILE SYSTEM ENTRY ----------------------------------------------------
    
    /// @class FileSystemEntry
    /// @brief Directory / file information utility
#   ifdef _WINDOWS
    template <bool _WideChar = true>
#   endif
    class FileSystemEntry final {
    public:
#     ifdef _WINDOWS
        using Type = FileSystemEntry<_WideChar>;
        using Path = typename std::conditional<_WideChar, std::wstring, std::string>::type; ///< Path string
        using PathChar = typename std::conditional<_WideChar, wchar_t, char>::type;         ///< Path character
        using Metadata = FileSystemEntryMetadata<_WideChar>; ///< Path metadata
#     else
        using Type = FileSystemEntry;
        using Path = std::string; ///< Path string
        using PathChar = char;    ///< Path character
        using Metadata = FileSystemEntryMetadata; ///< Path metadata
#     endif
      
      /// @brief Create relative path to current/active directory
      FileSystemEntry() = default;
      /// @brief Initialize existing directory/file path
      FileSystemEntry(const PathChar* path)
        : _entryPath(path), _entryType(getFileSystemEntryType(path)) { assert(path != nullptr); }
      /// @brief Initialize existing directory/file path
      explicit FileSystemEntry(const Path& path)
        : _entryPath(path), _entryType(getFileSystemEntryType(path.c_str())) {}
      
      /// @brief Initialize new/potential directory/file path
      FileSystemEntry(const PathChar* path, FileSystemEntryType targetType) noexcept
        : _entryPath(path), _entryType(targetType) { assert(path != nullptr); }
      /// @brief Initialize new/potential directory/file path
      FileSystemEntry(const Path& path, FileSystemEntryType targetType) noexcept
        : _entryPath(path), _entryType(targetType) {}
      /// @brief Initialize new/potential directory/file path
      FileSystemEntry(Path&& path, FileSystemEntryType targetType) noexcept
        : _entryPath(std::move(path)), _entryType(targetType) {}
      
      FileSystemEntry(const Type&) = default;
      FileSystemEntry(Type&&) noexcept = default;
      FileSystemEntry& operator=(const Type&) = default;
      FileSystemEntry& operator=(Type&&) noexcept = default;

      void swap(Type& rhs) noexcept {
        std::swap(this->_entryPath, rhs._entryPath);
        auto bufferType = this->_entryType;
        this->_entryType = rhs._entryType;
        rhs._entryType = bufferType;
      }

      // -- path metadata --
      
      inline const Path& path() const noexcept { return this->_entryPath; } ///< Get stored path value
      
      /// @brief Get type of stored path
      inline FileSystemPathType pathType() const noexcept { return getFileSystemPathType(this->_entryPath.c_str()); }
      /// @brief Get type of entry targeted by stored path
      inline FileSystemEntryType entryType() const noexcept { return this->_entryType; }
      /// @brief Verify if entry is hidden
      inline bool isHidden() noexcept { return isFileSystemEntryHidden(this->_entryPath.c_str()); }
      /// @brief Verify if stored path is existing / readable / writable / all
      inline bool hasAccessMode(FileSystemAccessMode mode) const noexcept { return verifyFileSystemAccessMode(this->_entryPath.c_str(), mode); }
      
      /// @brief Split stored path (on '/' or '\') into a vector of segments
      inline std::vector<Path> pathSegments() const { return getFileSystemPathSegments(this->_entryPath.c_str()); }
      /// @brief Read metadata of a file / directory
      inline bool readMetadata(Metadata& out) { return readFileSystemEntryMetadata(this->_entryPath, out); }
      
      // -- path creation/removal --
      
      /// @brief Open buffered file access (r/w/a/r+/w+/a+)
      /// @warning Check result.isOpen() to verify success
      inline pandora::io::FileHandle open(const PathChar* mode) noexcept { return (this->_entryType != FileSystemEntryType::directory) ? openFileEntry(this->_entryPath.c_str(), mode) : pandora::io::FileHandle{}; }
      /// @brief Create a new file / directory
      /// @returns 0 on success, or the value of errno on failure
      inline int create() noexcept { return (this->_entryType != FileSystemEntryType::directory) ? createFileEntry(this->_entryPath) : createDirectory(this->_entryPath); }
      /// @brief Delete an existing file / directory
      /// @returns 0 on success, or the value of errno on failure
      inline int remove() noexcept { return (this->_entryType != FileSystemEntryType::directory) ? removeFileEntry(this->_entryPath) : removeDirectory(this->_entryPath.c_str()); }
      
    private:
      Path _entryPath;
      FileSystemEntryType _entryType{ FileSystemEntryType::unknown };
    };

  }
}
#undef __if_constexpr
