/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Implementation included in file_system_io.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
// includes + namespaces: in file_system_io.cpp


// -- absolute path of current location --

pandora::io::SystemPath FileSystemLocationFinder::currentLocation() {
# ifndef _USE_NATIVE_FILESYSTEM // C++17
    std::error_code errorCode;
#   ifdef _WINDOWS
      pandora::io::SystemPath outPath = std::filesystem::current_path(errorCode).wstring();
#   else
      pandora::io::SystemPath outPath = std::filesystem::current_path(errorCode).u8string();
#   endif
    if (!errorCode)
      return outPath;
# endif
# ifdef _WINDOWS // native / fallback
    wchar_t buffer[MAX_PATH+1];
    return (GetCurrentDirectoryW(sizeof(buffer) / sizeof(wchar_t), buffer) > 0) ? pandora::io::SystemPath(buffer) : pandora::io::SystemPath{};
# else
    char* buffer = getenv("PWD");
    if (buffer != nullptr)
      return pandora::io::SystemPath(buffer);
    char curdir[_MAX_PATH_SIZE+1];
    return (getcwd(curdir, _MAX_PATH_SIZE+1) == 0) ? pandora::io::SystemPath(curdir) : pandora::io::SystemPath{};
# endif
}


// -- absolute path of user home directory --

pandora::io::SystemPath FileSystemLocationFinder::homeLocation() {
# if !defined(__ANDROID__) || defined(_WINDOWS) || defined(__APPLE__)
    return _getUserHomePath();
# else
    return pandora::io::SystemPath("files");
# endif
}


// -- absolute path(s) of standard user/app/system location --

#if !defined(__APPLE__) && !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
  // linux/unix - read customizable user directory path
  static inline std::string _getUserDataDirectoryPath(const char* prop, const char* defaultSuffix) {
    char buffer[_MAX_PATH_SIZE+1];

    // local directory settings
    std::string homeConfig = _getUserLocalConfigPath() + "/user-dirs.dirs";
    std::string localPropertyId = std::string("XDG_") + prop + "_DIR";
    size_t bufferLength = KeyValueFileReader::searchFileForValue(homeConfig.c_str(), localPropertyId.c_str(), buffer, _MAX_PATH_SIZE+1);
    if (bufferLength != 0u) {
      if (bufferLength >= 5 && memcmp(buffer, "$HOME", 5) == 0) // buffer has $HOME/path
        return _getUserHomePath() + &buffer[5];
      return std::string(buffer); // buffer has absolute path
    }
    // global directory settings
    if (KeyValueFileReader::searchFileForValue("/etc/xdg/user-dirs.defaults", prop, buffer, _MAX_PATH_SIZE+1) != 0u)
      return _getUserHomePath() + '/' + buffer; // buffer has relative path
    // default directory settings
    return _getUserHomePath() + defaultSuffix;
  }
#endif

// - get absolute path(s) of standard user/app/system location -
std::vector<pandora::io::SystemPath> FileSystemLocationFinder::standardLocation(FileSystemLocation location, const pandora::io::SystemPathChar* appDirLabel, uint32_t systemFlags) {
  std::vector<pandora::io::SystemPath> paths;

# if defined(_WINDOWS) // windows x86/x86_64
    switch (location) {
      case FileSystemLocation::home:      paths = { _findLocation(FOLDERID_Profile,  systemFlags, L"%USERPROFILE%") }; break;
      case FileSystemLocation::desktop:   paths = { _findLocation(FOLDERID_Desktop,  systemFlags, L"%USERPROFILE%\\Desktop") }; break;
      case FileSystemLocation::documents: paths = { _findLocation(FOLDERID_Documents,systemFlags, L"%USERPROFILE%\\Documents") }; break;
      case FileSystemLocation::pictures:  paths = { _findLocation(FOLDERID_Pictures, systemFlags, L"%USERPROFILE%\\Pictures") }; break;
      case FileSystemLocation::music:     paths = { _findLocation(FOLDERID_Music,  systemFlags, L"%USERPROFILE%\\Music") }; break;
      case FileSystemLocation::movies:    paths = { _findLocation(FOLDERID_Videos, systemFlags, L"%USERPROFILE%\\Videos") }; break;
      case FileSystemLocation::downloads: paths = { _findLocation(FOLDERID_Downloads,systemFlags, L"%USERPROFILE%\\Downloads") }; break;

      case FileSystemLocation::applications:   paths = { _findLocation(FOLDERID_StartMenu, systemFlags, L"%APPDATA%\\Microsoft\\Windows\\Start Menu"),
                                                         _findLocation(FOLDERID_CommonStartMenu, systemFlags, L"%ALLUSERSPROFILE%\\Microsoft\\Windows\\Start Menu") }; break;
      case FileSystemLocation::sharedApps:     paths = { _findLocation(FOLDERID_CommonStartMenu, systemFlags, L"%ALLUSERSPROFILE%\\Microsoft\\Windows\\Start Menu") }; break;
      case FileSystemLocation::appMenu:        paths = { _findLocation(FOLDERID_StartMenu, systemFlags, L"%APPDATA%\\Microsoft\\Windows\\Start Menu") }; break;
      case FileSystemLocation::fonts:          paths = { _findLocation(FOLDERID_Fonts, systemFlags, L"%windir%\\Fonts") }; break;
        
      case FileSystemLocation::appData:        paths = { _findLocation(FOLDERID_RoamingAppData, systemFlags, L"%APPDATA%") + L"\\" + appDirLabel,
                                                         _findLocation(FOLDERID_ProgramData, systemFlags, L"%ALLUSERSPROFILE%") + L"\\" + appDirLabel,
                                                         currentLocation() + L"\\data" }; break;
      case FileSystemLocation::localAppData:   paths = { _findLocation(FOLDERID_LocalAppData,systemFlags, L"%LOCALAPPDATA%") + L"\\" + appDirLabel,
                                                         _findLocation(FOLDERID_ProgramData, systemFlags, L"%ALLUSERSPROFILE%") + L"\\" + appDirLabel,
                                                         currentLocation() + L"\\data" }; break;
      case FileSystemLocation::appConfig:      paths = { _findLocation(FOLDERID_LocalAppData,systemFlags, L"%LOCALAPPDATA%") + L"\\" + appDirLabel,
                                                         _findLocation(FOLDERID_ProgramData, systemFlags, L"%ALLUSERSPROFILE%") + L"\\" + appDirLabel }; break;
      case FileSystemLocation::commonAppConfig:paths = { _findLocation(FOLDERID_LocalAppData,systemFlags, L"%LOCALAPPDATA%"),
                                                         _findLocation(FOLDERID_ProgramData, systemFlags, L"%ALLUSERSPROFILE%") }; break;
      case FileSystemLocation::cache:          paths = { _findLocation(FOLDERID_LocalAppData,systemFlags, L"%LOCALAPPDATA%") + L"\\" + appDirLabel + L"\\cache" }; break;
      case FileSystemLocation::localCache:     paths = { _findLocation(FOLDERID_LocalAppData,systemFlags, L"%LOCALAPPDATA%") + L"\\cache" }; break;
      case FileSystemLocation::sharedCache:    paths = { _findLocation(FOLDERID_ProgramData, systemFlags, L"%ALLUSERSPROFILE%") + L"\\cache" }; break;
      case FileSystemLocation::temp:           paths = { _findLocation(FOLDERID_LocalAppData,systemFlags, L"%LOCALAPPDATA%") + L"\\Temp" }; break;

#     if defined(_WIN64) || defined(WIN64) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(_M_X64) || defined(_M_I64) || defined(_M_IX64) || defined(_M_AMD64) || defined(__IA64__) || defined(_IA64_)
        case FileSystemLocation::system:            paths = { _findLocation(FOLDERID_SystemX86, systemFlags, L"%windir%\\system32"),
                                                              _findLocation(FOLDERID_System,  systemFlags, L"%windir%\\syswow64"),
                                                              _findLocation(FOLDERID_Windows, systemFlags, L"%windir%") }; break;
        case FileSystemLocation::localBinaries:     paths = { _findLocation(FOLDERID_UserProgramFiles, systemFlags, L"%LOCALAPPDATA%\\Programs") }; break;
        case FileSystemLocation::sharedBinaries:    paths = { _findLocation(FOLDERID_ProgramFiles, systemFlags, L"%ProgramFiles%"),
                                                              _findLocation(FOLDERID_ProgramFilesX86, systemFlags, L"%ProgramFiles% (x86)") }; break;
        case FileSystemLocation::sharedLibraries:   paths = { _findLocation(FOLDERID_System, systemFlags, L"%windir%\\syswow64"),
                                                              _findLocation(FOLDERID_SystemX86, systemFlags, L"%windir%\\system32"),
                                                              _findLocation(FOLDERID_ProgramFilesCommon, systemFlags, L"%ProgramFiles%\\Common Files"),
                                                              _findLocation(FOLDERID_ProgramFilesCommonX86, systemFlags, L"%ProgramFiles% (x86)\\Common Files") }; break;
        case FileSystemLocation::commonBinaryFiles: paths = { _findLocation(FOLDERID_ProgramFilesCommon, systemFlags, L"%ProgramFiles%\\Common Files"),
                                                              _findLocation(FOLDERID_ProgramFilesCommonX86, systemFlags, L"%ProgramFiles% (x86)\\Common Files") }; break;
#     else
        case FileSystemLocation::system:            paths = { _findLocation(FOLDERID_System,  systemFlags, L"%windir%\\system32"),
                                                              _findLocation(FOLDERID_Windows, systemFlags, L"%windir%") }; break;
        case FileSystemLocation::localBinaries:     paths = { _findLocation(FOLDERID_UserProgramFiles, systemFlags, L"%LOCALAPPDATA%\\Programs") }; break;
        case FileSystemLocation::sharedBinaries:    paths = { _findLocation(FOLDERID_ProgramFiles, systemFlags, L"%ProgramFiles%") }; break;
        case FileSystemLocation::sharedLibraries:   paths = { _findLocation(FOLDERID_System, systemFlags, L"%windir%\\system32"),
                                                              _findLocation(FOLDERID_ProgramFilesCommon, systemFlags, L"%ProgramFiles%\\Common Files") }; break;
        case FileSystemLocation::commonBinaryFiles: paths = { _findLocation(FOLDERID_ProgramFilesCommon, systemFlags, L"%ProgramFiles%\\Common Files") }; break;
#     endif
        
      case FileSystemLocation::printers: { 
        pandora::io::SystemPath path;
        if (_findLocation(FOLDERID_PrintersFolder, path, systemFlags)) 
          paths = { path }; 
        break;
      }
      case FileSystemLocation::trash: { 
        pandora::io::SystemPath path;
        if (_findLocation(FOLDERID_RecycleBinFolder, path, systemFlags)) 
          paths = { path }; 
        break;
      }
      default: break;
    }
    
# elif defined(__APPLE__)
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE // iOS
      switch (location) {
        case FileSystemLocation::home:      paths = { _getUserHomePath() }; break;
        case FileSystemLocation::desktop:   paths = { "Documents/Desktop" }; break;
        case FileSystemLocation::documents: paths = { "Documents" }; break;
        case FileSystemLocation::pictures:  paths = { "Documents/Pictures" }; break;
        case FileSystemLocation::music:     paths = { "Documents/Music" }; break;
        case FileSystemLocation::movies:    paths = { "Documents/Movies" }; break;
        case FileSystemLocation::downloads: paths = { "Documents/Downloads" }; break;
        case FileSystemLocation::appMenu:   paths = { "/System/Library/CoreServices" }; break;
        case FileSystemLocation::fonts:     paths = { "Library/Fonts", "/System/Library/Fonts" }; break;
                
        case FileSystemLocation::appData:
        case FileSystemLocation::localAppData:   paths = { "Library/Application Support" }; break;
        case FileSystemLocation::appConfig:      paths = { std::string("Library/Preferences/") + appDirLabel }; break;
        case FileSystemLocation::commonAppConfig:paths = { "Library/Preferences" }; break;
        case FileSystemLocation::cache:
        case FileSystemLocation::localCache:     paths = { "Library/Caches" }; break;
        case FileSystemLocation::sharedCache:    paths = { "Documents/Inbox" }; break;
        case FileSystemLocation::temp:           paths = { "tmp" }; break;

        case FileSystemLocation::sharedLibraries:   paths = { "lib", "Library/Frameworks", "/System/Library/Frameworks" }; break;
        case FileSystemLocation::commonBinaryFiles: paths = { "Library" }; break;

        case FileSystemLocation::printers: paths = { "Library/Printers/PPDs" }; break;
        default: break;
      }
#   else // mac OS X
      switch (location) {
        case FileSystemLocation::home:      paths = { _getUserHomePath() }; break;
        case FileSystemLocation::desktop:   paths = _findLocation(SYSDIR_DIRECTORY_DESKTOP, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Desktop"); break;
        case FileSystemLocation::documents: paths = _findLocation(SYSDIR_DIRECTORY_DOCUMENT, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Documents"); break;
        case FileSystemLocation::pictures:  paths = _findLocation(SYSDIR_DIRECTORY_PICTURES, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Pictures"); break;
        case FileSystemLocation::music:     paths = _findLocation(SYSDIR_DIRECTORY_MUSIC, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Music"); break;
        case FileSystemLocation::movies:    paths = _findLocation(SYSDIR_DIRECTORY_MOVIES, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Movies"); break;
        case FileSystemLocation::downloads: paths = _findLocation(SYSDIR_DIRECTORY_DOWNLOADS, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Downloads"); break;

        case FileSystemLocation::applications: paths = _findLocation(SYSDIR_DIRECTORY_ALL_APPLICATIONS, SYSDIR_DOMAIN_MASK__SYSTEM_USER_LOCAL, _getUserHomePath() + "/Applications"); break;
        case FileSystemLocation::sharedApps:   paths = _findLocation(SYSDIR_DIRECTORY_ALL_APPLICATIONS, SYSDIR_DOMAIN_MASK_ALL, _getUserHomePath() + "/Applications"); break;
        case FileSystemLocation::appMenu:      paths = _findLocation(SYSDIR_DIRECTORY_CORESERVICE, SYSDIR_DOMAIN_MASK_ALL, "/System/Library/CoreServices"); break;
        case FileSystemLocation::fonts:        paths = _findLocation(SYSDIR_DIRECTORY_ALL_LIBRARIES, SYSDIR_DOMAIN_MASK_ALL, "/System/Library", "/Fonts"); break;
        
        case FileSystemLocation::appData:        paths = _findLocation(SYSDIR_DIRECTORY_ALL_LIBRARIES, SYSDIR_DOMAIN_MASK_ALL, _getUserHomePath() + "/Library/Application Support", std::string("/") + appDirLabel); paths.emplace_back("../Resources"); break;
        case FileSystemLocation::localAppData:   paths = _findLocation(SYSDIR_DIRECTORY_ALL_LIBRARIES, SYSDIR_DOMAIN_MASK_ALL, _getUserHomePath() + "/Library/Application Support"); paths.emplace_back("../Resources"); break;
        case FileSystemLocation::appConfig:      paths = _findLocation(SYSDIR_DIRECTORY_LIBRARY, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Library", std::string("/Preferences/") + appDirLabel); break;
        case FileSystemLocation::commonAppConfig:paths = _findLocation(SYSDIR_DIRECTORY_LIBRARY, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Library", "/Preferences"); break;
        case FileSystemLocation::cache:          paths = _findLocation(SYSDIR_DIRECTORY_CACHES, SYSDIR_DOMAIN_MASK__USER_LOCAL, _getUserHomePath() + "/Library/Caches", std::string("/") + appDirLabel); break;
        case FileSystemLocation::localCache:     paths = _findLocation(SYSDIR_DIRECTORY_CACHES, SYSDIR_DOMAIN_MASK__USER_LOCAL, _getUserHomePath() + "/Library/Caches"); break;
        case FileSystemLocation::sharedCache:    paths = _findLocation(SYSDIR_DIRECTORY_CACHES, SYSDIR_DOMAIN_MASK_LOCAL, "/Library/Caches", std::string("/") + appDirLabel); break;
        case FileSystemLocation::temp: {
          char* envBuffer = getenv("TMPDIR");
          paths.emplace_back((envBuffer != nullptr) ? envBuffer : "$TMPDIR");
          break;
        }

        case FileSystemLocation::system:           paths = _findLocation(SYSDIR_DIRECTORY_ADMIN_APPLICATION, SYSDIR_DOMAIN_MASK__SYSTEM_LOCAL); paths.emplace_back("/System"); break;
        case FileSystemLocation::localBinaries:    paths = _findLocation(SYSDIR_DIRECTORY_ALL_APPLICATIONS, SYSDIR_DOMAIN_MASK_USER, "/System/Library/CoreServices"); break;
        case FileSystemLocation::sharedBinaries:   paths = _findLocation(SYSDIR_DIRECTORY_ALL_APPLICATIONS, SYSDIR_DOMAIN_MASK__SYSTEM_NET_LOCAL, "/System/Library/CoreServices"); break;
        case FileSystemLocation::sharedLibraries:  paths = { _getUserHomePath() + "/lib", "/usr/local/lib", "/usr/lib" }; break;
        case FileSystemLocation::commonBinaryFiles:paths = _findLocation(SYSDIR_DIRECTORY_ALL_LIBRARIES, SYSDIR_DOMAIN_MASK__SYSTEM_LOCAL, "/Library"); break;

        case FileSystemLocation::printers: paths = _findLocation(SYSDIR_DIRECTORY_PRINTER_DESCRIPTION, SYSDIR_DOMAIN_MASK_ALL); break;
        case FileSystemLocation::trash:    paths = _findLocation(SYSDIR_DIRECTORY_DESKTOP, SYSDIR_DOMAIN_MASK_USER, _getUserHomePath() + "/Desktop", "/.Trash"); break;
        default: break;
      }
#   endif
    
# elif defined(__ANDROID__) // android
    switch (location) {
      case FileSystemLocation::home:      paths = { "files" }; break;
      case FileSystemLocation::desktop:   paths = { "files" }; break;
      case FileSystemLocation::documents: paths = { "files/Documents", "/storage/sdcard0/Documents", std::string("/storage/sdcard0/") + appDirLabel + "/Documents" }; break;
      case FileSystemLocation::pictures:  paths = { "files/Pictures", "/storage/sdcard0/Pictures" }; break;
      case FileSystemLocation::music:     paths = { "files/Music", "/storage/sdcard0/Music" }; break;
      case FileSystemLocation::movies:    paths = { "files/Movies", "/storage/sdcard0/Movies" }; break;
      case FileSystemLocation::downloads: paths = { "files/Downloads", "/storage/sdcard0/Downloads", std::string("/storage/sdcard0/") + appDirLabel + "/Downloads" }; break;
      case FileSystemLocation::fonts:     paths = { "/system/fonts" }; break;
                
      case FileSystemLocation::appData:
      case FileSystemLocation::localAppData:   paths = { "files" }; break;
      case FileSystemLocation::appConfig:      paths = { "files/settings/" }; break;
      case FileSystemLocation::commonAppConfig:paths = { "/storage/sdcard0/settings" }; break;
      case FileSystemLocation::cache:          paths = { "cache" }; break;
      case FileSystemLocation::localCache:     paths = { "cache", std::string("/storage/sdcard0/") + appDirLabel + "/cache" }; break;
      case FileSystemLocation::sharedCache:    paths = { "/storage/sdcard0/cache" }; break;
      case FileSystemLocation::temp:           paths = { "cache" }; break;

      case FileSystemLocation::sharedLibraries:   paths = { "lib" }; break;
      default: break;
    }
    
# elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix) // linux/unix
    switch (location) {
      case FileSystemLocation::home:      paths = { _getUserHomePath() }; break;
      case FileSystemLocation::desktop:   paths = { _getUserDataDirectoryPath("DESKTOP", "/Desktop") }; break;
      case FileSystemLocation::documents: paths = { _getUserDataDirectoryPath("DOCUMENTS", "/Documents") }; break;
      case FileSystemLocation::pictures:  paths = { _getUserDataDirectoryPath("PICTURES", "/Pictures") }; break;
      case FileSystemLocation::music:     paths = { _getUserDataDirectoryPath("MUSIC", "/Music") }; break;
      case FileSystemLocation::movies:    paths = { _getUserDataDirectoryPath("VIDEOS", "/Videos") }; break;
      case FileSystemLocation::downloads: paths = { _getUserDataDirectoryPath("DOWNLOAD", "/Downloads") }; break;

      case FileSystemLocation::applications: paths = { _getUserHomePath() + "/.local/share/applications", "/usr/local/share/applications", "/usr/share/applications" }; break;
      case FileSystemLocation::sharedApps:   paths = { "/usr/local/share/applications", "/usr/share/applications" }; break;
      case FileSystemLocation::appMenu:      paths = { _getUserHomePath() + "/.local/share/applications" }; break;
      case FileSystemLocation::fonts:        paths = { "/usr/share/fonts", "/usr/local/share/fonts", _getUserHomePath() + "/.fonts", _getUserHomePath() + "/.local/share/fonts" }; break;

      case FileSystemLocation::appData:        paths = { _getUserHomePath() + "/.local/share/" + appDirLabel, std::string("/usr/local/share/") + appDirLabel, std::string("/usr/share/") + appDirLabel }; break;
      case FileSystemLocation::localAppData:   paths = { _getUserHomePath() + "/.local/share", "/usr/local/share", "/usr/share" }; break;
      case FileSystemLocation::appConfig:      paths = { _getUserHomePath() + "/.config/" + appDirLabel, std::string("/etc/xdg/") + appDirLabel }; break;
      case FileSystemLocation::commonAppConfig:paths = { _getUserHomePath() + "/.config", "/etc/xdg" }; break;
      case FileSystemLocation::cache:          paths = { _getUserHomePath() + "/.cache/" + appDirLabel }; break;
      case FileSystemLocation::localCache:     paths = { _getUserHomePath() + "/.cache" }; break;
      case FileSystemLocation::sharedCache:    paths = { "/root" }; break;
      case FileSystemLocation::temp:           paths = { "/tmp", "/var/tmp" }; break;

      case FileSystemLocation::system:            paths = { "/bin", "/lib", "/sys" }; break;
      case FileSystemLocation::localBinaries:     paths = { "/usr/local/bin" }; break;
      case FileSystemLocation::sharedBinaries:    paths = { "/bin", "/usr/bin", "/usr/share/bin" }; break;
      case FileSystemLocation::sharedLibraries:   paths = { "/lib", "/usr/lib", "/usr/local/lib", "/usr/share/lib" }; break;
      case FileSystemLocation::commonBinaryFiles: paths = { "/usr/lib", "/usr/share/lib" }; break;

      case FileSystemLocation::printers: paths = { "/dev" }; break;
      case FileSystemLocation::trash:    paths = { _getUserHomePath() + "/.local/share/Trash" }; break;
      default: break;
    }
# endif
  return paths;
}
