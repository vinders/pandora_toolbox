/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Apple compatibility layer for CPU affinity system calls
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS) && (defined(__ANDROID__) || defined(__APPLE__))
# if defined(__ANDROID__)
#   include <sys/syscall.h>
#   include <unistd.h>
#   include <pthread.h>
# elif defined(__APPLE__)
#   include <cstdint>
#   include <cstddef>
#   include <sys/types.h>
#   include <sys/sysctl.h>
#   include <pthread.h>
#   include <unistd.h>
#   include <mach/thread_policy.h>
#   include <mach/thread_act.h>
#   define SYSCTL_CORE_COUNT   "machdep.cpu.core_count"
# endif

# if defined(__APPLE__)
    /// @brief CPU affinity structure
    typedef struct cpu_set {
      uint32_t count;
    } cpu_set_t;

    // -- bitmask macros --

    static inline void CPU_ZERO(cpu_set_t* mask) { mask->count = 0; }
    static inline void CPU_SET(int32_t core, cpu_set_t* mask) { mask->count |= (1 << core); }
    static inline int CPU_ISSET(int32_t core, cpu_set_t* mask) { return (mask->count & (1 << core)); }
# endif

  // -- process affinity --

  inline int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
#   if defined(__APPLE__)
      int32_t coreCount = 0;
      size_t coreCountSize = sizeof(coreCount);
      if (sysctlbyname(SYSCTL_CORE_COUNT, &coreCount, &coreCountSize, 0, 0))
        return -1;
      if (coreCount > 8*static_cast<int32_t>(cpusetsize))
        coreCount = 8*static_cast<int32_t>(cpusetsize);

      mask->count = 0;
      for (int32_t i = 0; i < coreCount; ++i)
        mask->count |= (1 << i);
      return 0;
#   else
      return syscall(__NR_sched_getaffinity, pid, cpusetsize, mask);
#   endif
  }

  inline int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* mask) {
#   if defined(__APPLE__)
      return -1; // not supported on OSX/iOS
#   else
      return syscall(__NR_sched_setaffinity, pid, cpusetsize, mask);
#   endif
  }

  // -- thread affinity --

  inline int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t* mask) {
#   if defined(__APPLE__)
      for (int32_t core = 0; core < 8*static_cast<int32_t>(cpusetsize); ++core) {
        if (CPU_ISSET(core, mask)) {
          thread_affinity_policy_data_t policy = { core };
          thread_port_t mach_thread = pthread_mach_thread_np(thread);
          thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
        }
      }
      return 0;
#   else
      return -1; // not supported on android
#   endif
  }

#endif
