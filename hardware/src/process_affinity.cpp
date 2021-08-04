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
#include <cstdint>
#include <thread>

#ifdef _WINDOWS
# include <system/api/windows_api.h>

#else
# include <system/operating_system.h>
# if _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_LINUX || _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_UNIX || _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_MAC_OSX || _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_ANDROID
#   include <cstddef>
#   include <sys/types.h>
#   include <unistd.h>
#   include <pthread.h>
#   define __P_USE_POSIX_PTHREAD
# endif
# include "hardware/_private/_process_affinity_binding.h"
#endif

#include "hardware/process_affinity.h"

using namespace pandora::hardware;

// -- process affinity --

bool pandora::hardware::getCurrentProcessAffinity(int32_t& cpuCoresBitmask) noexcept {
# if defined(_WINDOWS)
    DWORD_PTR appAffinityMask = 0u;
	DWORD_PTR systemAffinityMask = 0u;
    if (GetProcessAffinityMask(::GetCurrentProcess(), &appAffinityMask, &systemAffinityMask) != 0u) {
      cpuCoresBitmask = static_cast<int32_t>(appAffinityMask);
      return true;
    }
# elif defined(__P_USE_POSIX_PTHREAD)
    cpu_set_t cpuSet;
    cpuCoresBitmask = 0u;
    if (sched_getaffinity(getpid(), sizeof(cpu_set_t), &cpuSet) == 0) {
      for (int core = 0; core < 32; ++core)
        if (CPU_ISSET(core, &cpuSet))
          cpuCoresBitmask |= (1 << core);
    }
# endif
  return false;
}

bool pandora::hardware::setCurrentProcessAffinity(int32_t cpuCoresBitmask) noexcept {
# if defined(_WINDOWS)
    return (SetProcessAffinityMask(::GetCurrentProcess(), static_cast<DWORD_PTR>(cpuCoresBitmask)) != 0u);
# elif defined(__P_USE_POSIX_PTHREAD)
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (int core = 0; core < 32; ++core)
      if (cpuCoresBitmask & (1 << core))
        CPU_SET(core, &cpuSet);
    return (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuSet) == 0);
# else
    return false;
# endif
}

// -- thread affinity --

bool pandora::hardware::setCurrentThreadAffinity(int32_t cpuCoresBitmask) noexcept {
# if defined(_WINDOWS)
    return (SetThreadAffinityMask(::GetCurrentThread(), static_cast<DWORD_PTR>(cpuCoresBitmask)) != 0u);
# elif defined(__P_USE_POSIX_PTHREAD)
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (int core = 0; core < 32; ++core)
      if (cpuCoresBitmask & (1 << core))
        CPU_SET(core, &cpuSet);
    return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet) == 0);
# else
    return false;
# endif
}

bool pandora::hardware::setThreadAffinity(std::thread& thread, int32_t cpuCoresBitmask) noexcept {
# if defined(_WINDOWS)
    return (SetThreadAffinityMask((HANDLE)thread.native_handle(), static_cast<DWORD_PTR>(cpuCoresBitmask)) != 0u);
# elif defined(__P_USE_POSIX_PTHREAD)
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (int core = 0; core < 32; ++core)
      if (cpuCoresBitmask & (1 << core))
        CPU_SET(core, &cpuSet);
    return (pthread_setaffinity_np((pthread_t)thread.native_handle(), sizeof(cpu_set_t), &cpuSet) == 0);
# else
    return false;
# endif
}
