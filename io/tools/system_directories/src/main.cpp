/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
List common system-specific directories
*******************************************************************************/
#include <cstdio>
#include <io/file_system_locations.h>

using namespace pandora::io;

// Display list of directories
void displayDirectories(const std::string& title, const std::vector<SystemPath>& directories) {
  char offset[32];
  for (int i = 0; i < title.size(); ++i)
    offset[i] = ' ';
  offset[title.size()] = 0;
  
  printf(" %s  ", title.c_str());
  if (!directories.empty()) {
#     ifdef _WINDOWS
        printf("%S\n", directories.begin()->c_str());
#     else
        printf("%s\n", directories.begin()->c_str());
#     endif
    
    for (auto it = directories.begin() + 1; it != directories.end(); ++it) {
#     ifdef _WINDOWS
        printf(" %s  %S\n", offset, it->c_str());
#     else
        printf("%s%s\n", offset, it->c_str());
#     endif
    }
  }
  else
    printf("-\n");
}

// CPU analysis
int main() {
# ifdef _WINDOWS
    std::wstring appName = L"system_directories";
# else
    std::string appName = "system_directories";
# endif
  printf("\n COMMON SYSTEM-SPECIFIC DIRECTORIES\n"
         "____________________________________________________________\n\n");

  printf("-- USER DATA -- --------------------------------------------\n\n");
  displayDirectories("User home     ", FileSystemLocationFinder::standardLocation(FileSystemLocation::home, appName.c_str()));
  displayDirectories("Desktop       ", FileSystemLocationFinder::standardLocation(FileSystemLocation::desktop, appName.c_str()));
  displayDirectories("Documents     ", FileSystemLocationFinder::standardLocation(FileSystemLocation::documents, appName.c_str()));
  displayDirectories("Pictures      ", FileSystemLocationFinder::standardLocation(FileSystemLocation::pictures, appName.c_str()));
  displayDirectories("Music         ", FileSystemLocationFinder::standardLocation(FileSystemLocation::music, appName.c_str()));
  displayDirectories("Videos        ", FileSystemLocationFinder::standardLocation(FileSystemLocation::movies, appName.c_str()));
  displayDirectories("Downloads     ", FileSystemLocationFinder::standardLocation(FileSystemLocation::downloads, appName.c_str()));
  
  printf("\n-- APPLICATIVE -- ------------------------------------------\n\n");
  displayDirectories("Current dir   ", std::vector<pandora::io::SystemPath>{ FileSystemLocationFinder::currentLocation() });
  displayDirectories("Applications  ", FileSystemLocationFinder::standardLocation(FileSystemLocation::applications, appName.c_str()));
  displayDirectories("Shared apps   ", FileSystemLocationFinder::standardLocation(FileSystemLocation::sharedApps, appName.c_str()));
  displayDirectories("App menus     ", FileSystemLocationFinder::standardLocation(FileSystemLocation::appMenu, appName.c_str()));
  displayDirectories("System fonts  ", FileSystemLocationFinder::standardLocation(FileSystemLocation::fonts, appName.c_str()));

  printf("\n-- APPLICATION DATA -- -------------------------------------\n\n");
  displayDirectories("Roaming data  ", FileSystemLocationFinder::standardLocation(FileSystemLocation::appData, appName.c_str()));
  displayDirectories("Local data    ", FileSystemLocationFinder::standardLocation(FileSystemLocation::localAppData, appName.c_str()));
  displayDirectories("App config    ", FileSystemLocationFinder::standardLocation(FileSystemLocation::appConfig, appName.c_str()));
  displayDirectories("Shared config ", FileSystemLocationFinder::standardLocation(FileSystemLocation::commonAppConfig, appName.c_str()));
  displayDirectories("User cache    ", FileSystemLocationFinder::standardLocation(FileSystemLocation::cache, appName.c_str()));
  displayDirectories("Local cache   ", FileSystemLocationFinder::standardLocation(FileSystemLocation::localCache, appName.c_str()));
  displayDirectories("Shared cache  ", FileSystemLocationFinder::standardLocation(FileSystemLocation::sharedCache, appName.c_str()));
  displayDirectories("Temporary     ", FileSystemLocationFinder::standardLocation(FileSystemLocation::temp, appName.c_str()));
  
  printf("\n-- SYSTEM & BINARIES -- ------------------------------------\n\n");
  displayDirectories("System        ", FileSystemLocationFinder::standardLocation(FileSystemLocation::system, appName.c_str()));
  displayDirectories("Local binaries", FileSystemLocationFinder::standardLocation(FileSystemLocation::localBinaries, appName.c_str()));
  displayDirectories("Shared bin    ", FileSystemLocationFinder::standardLocation(FileSystemLocation::sharedBinaries, appName.c_str()));
  displayDirectories("Common bin    ", FileSystemLocationFinder::standardLocation(FileSystemLocation::commonBinaryFiles, appName.c_str()));
  displayDirectories("Shared libs   ", FileSystemLocationFinder::standardLocation(FileSystemLocation::sharedLibraries, appName.c_str()));
  displayDirectories("Printers links", FileSystemLocationFinder::standardLocation(FileSystemLocation::printers, appName.c_str()));
  displayDirectories("Trash         ", FileSystemLocationFinder::standardLocation(FileSystemLocation::trash, appName.c_str()));
  
  printf("\n____________________________________________________________\n\n"
         " Press enter to exit...");
  getchar();
  return 0;
}
