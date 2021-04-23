/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Local hardware specifications analyzer (CPU, monitors, display adapters)
*******************************************************************************/
#include <cstdio>
#include <hardware/cpu_specs.h>
#include <hardware/display_monitor.h>

#if defined(__ANDROID__)
# include <stdexcept>
# include <android/log.h>
# include <system/api/android_app.h>
# define printf(...) __android_log_print(ANDROID_LOG_INFO, "-", __VA_ARGS__)
# ifndef LOGE
#   define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , ">", __VA_ARGS__)
# endif
#endif

using namespace pandora::hardware;

// Clear all traces in console
void clearScreen() {
# ifdef _WINDOWS
    system("cls");
# elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
    if (system("clear") == -1)
      printf("\n____________________________________________________________\n");
# endif
}

// Display CPU specs
void displaySpecs(const CpuSpecs& specs) {
  std::string instructionSets;
  for (const auto& instSet : specs.getAvailableCpuInstructionSets()) {
    if (instSet != CpuInstructionSet::cpp)
      instructionSets += std::string(" - ") + toString(instSet) + "\n";
  }

  std::string architecture = toString(specs.archType()).c_str();
  if (pandora::system::getCpuRegisterSize() > 32)
    architecture += std::string("_") + std::to_string(pandora::system::getCpuRegisterSize());

  clearScreen();
  printf("\n HARDWARE SPECIFICATIONS ANALYZER\n"
         "____________________________________________________________\n\n"
         " CPU vendor:   %s\n CPU model:    %s\n Architecture: %s\n\n"
         " Physical cores:  %u\n Logical cores:   %u\n Hyper-threading: %s\n\n"
         " Extended instruction sets :\n%s\n",
        specs.vendor().c_str(), specs.brand().c_str(), architecture.c_str(),
        specs.physicalCores(), specs.logicalCores(), specs.isHyperThreadingCapable() ? "true" : "false",
        instructionSets.c_str());

  DisplayMonitor::setDpiAwareness(true);
  std::vector<DisplayMonitor> monitors = DisplayMonitor::listAvailableMonitors();
  printf(" Display monitors:\n");
  for (auto& it : monitors) {
    DisplayMode mode = it.getDisplayMode();

#   if defined(_WINDOWS)
      std::wstring adapterName = it.adapterName();
      printf(" - %S: %ux%u (work area:%ux%u) %s\n   %S (%S)\n   Display mode: %ux%u:%u @%.2fHz\n", it.attributes().id.c_str(),
            it.attributes().screenArea.width, it.attributes().screenArea.height,
            it.attributes().workArea.width, it.attributes().workArea.height,
            it.attributes().isPrimary ? "- primary" : " ",
            it.attributes().description.c_str(), adapterName.c_str(),
            mode.width, mode.height, mode.bitDepth, (float)mode.refreshRate/1000.0f);
#   else
      std::string adapterName = it.adapterName();
      if (adapterName.empty())
        adapterName = "-";
      printf(" - %s: %ux%u (work area:%ux%u) %s\n   %s (%s)\n   Display mode: %ux%u:%u @%.2fHz\n", it.attributes().id.c_str(),
            it.attributes().screenArea.width, it.attributes().screenArea.height,
            it.attributes().workArea.width, it.attributes().workArea.height,
            it.attributes().isPrimary ? "- primary" : " ",
            it.attributes().description.c_str(), adapterName.c_str(),
            mode.width, mode.height, mode.bitDepth, (float)mode.refreshRate/1000.0f);
#   endif
  }
  printf("\n____________________________________________________________\n\n");
}

// ---

#if defined(__ANDROID__)
  // CPU analysis - entry point for Android
  void android_main(struct android_app* state) {
    try {
      pandora::system::AndroidApp::instance().init(state);
      CpuSpecs cpuAnalyzer(CpuSpecs::SpecMode::all);
      displaySpecs(cpuAnalyzer);
    }
    catch (const std::exception& exc) { LOGE("%s", exc.what()); }
  }
  
#else
  // CPU analysis - entry point
  int main() {
    CpuSpecs cpuAnalyzer(CpuSpecs::SpecMode::all);
    displaySpecs(cpuAnalyzer);
    
    printf(" Press enter to exit...");
    getchar();
    return 0;
  }
#endif
