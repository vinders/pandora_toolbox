/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
------------------------------------------------------------------------
Description : set process/thread affinity with CPU
Functions : getCurrentProcessAffinity, setCurrentProcessAffinity,
            setCurrentThreadAffinity, setThreadAffinity
*******************************************************************************/
#pragma once

#include <cstdint>
#include <thread>

namespace pandora { 
  namespace hardware {
    // -- process affinity --
    
    /// @brief Get processor affinity bitmask of current process. Each bit (bit 0-31) set to 1 represents a CPU core (core 1-32) affinity.
    /// @warning Requires privileges on Apple systems.
    bool getCurrentProcessAffinity(int32_t& cpuCoresBitmask) noexcept;
    
    /// @brief Set processor affinity of current process. Each bit (bit 0-31) set to 1 represents a CPU core (core 1-32) affinity.
    /// @warning Not supported on Apple systems.
    ///          May require privileges on linux/unix systems.
    bool setCurrentProcessAffinity(int32_t cpuCoresBitmask) noexcept;
    
    // -- thread affinity --
    
    /// @brief Set processor affinity of current thread. Each bit (bit 0-31) set to 1 represents a CPU core (core 1-32) affinity.
    /// @warning The thread affinity mask should only be a subset of the current process affinity mask.
    ///          Not supported on Android systems.
    bool setCurrentThreadAffinity(int32_t cpuCoresBitmask) noexcept;

    /// @brief Set processor affinity of a specific thread. Each bit (bit 0-31) set to 1 represents a CPU core (core 1-32) affinity.
    /// @warning The thread affinity mask should only be a subset of the current process affinity mask.
    ///          Not supported on Android systems.
    bool setThreadAffinity(std::thread& thread, int32_t cpuCoresBitmask) noexcept;
  }
}
