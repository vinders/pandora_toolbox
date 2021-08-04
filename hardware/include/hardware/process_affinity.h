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
