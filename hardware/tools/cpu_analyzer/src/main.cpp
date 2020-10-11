/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Local CPU specifications analyzer
*******************************************************************************/
#include <cstdio>
#include <hardware/cpu_specs.h>

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
  printf("\n CPU SPECIFICATIONS ANALYZER\n"
         "____________________________________________________________\n\n"
         " CPU vendor:   %s\n CPU model:    %s\n Architecture: %s\n\n"
         " Physical cores:  %u\n Logical cores:   %u\n Hyper-threading: %s\n\n"
         " Extended instruction sets :\n%s\n"
         "____________________________________________________________\n\n",
        specs.vendor().c_str(), specs.brand().c_str(), architecture.c_str(),
        specs.physicalCores(), specs.logicalCores(), specs.isHyperThreadingCapable() ? "true" : "false",
        instructionSets.c_str());
}

// CPU analysis
int main() {
  CpuSpecs cpuAnalyzer(CpuSpecs::SpecMode::all);
  displaySpecs(cpuAnalyzer);
  
  printf(" Press enter to exit...");
  getchar();
  return 0;
}
