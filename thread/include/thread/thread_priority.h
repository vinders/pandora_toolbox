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
Description : set thread priority for scheduler
Functions : setCurrentThreadPriority, setThreadPriority
*******************************************************************************/
#pragma once

#include <thread>
#ifdef _WINDOWS
# include <system/api/windows_api.h>
# define THREAD_PRIORITY_UNKNOWN THREAD_PRIORITY_NORMAL
# ifdef __MINGW32__
#   include <pthread.h>
#   define __P_USE_POSIX_PTHREAD
# endif

#else
# include <system/operating_system.h>
# if _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_MAC_OSX || _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_UNIX || _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_LINUX || _SYSTEM_OPERATING_SYSTEM == _SYSTEM_OS_ANDROID
#   include <sys/types.h>
#   include <unistd.h>
#   include <pthread.h>
#   define __P_USE_POSIX_PTHREAD
# endif
# define THREAD_PRIORITY_UNKNOWN       0
# define THREAD_PRIORITY_IDLE          1
# define THREAD_PRIORITY_LOWEST        32
# define THREAD_PRIORITY_BELOW_NORMAL  40
# define THREAD_PRIORITY_NORMAL        48
# define THREAD_PRIORITY_ABOVE_NORMAL  56
# define THREAD_PRIORITY_HIGHEST       64
# define THREAD_PRIORITY_TIME_CRITICAL 96
#endif

namespace pandora { 
  namespace thread {
    /// @brief Scheduler priority for threads
    enum class ThreadPriority : int {
      reset    = THREAD_PRIORITY_UNKNOWN,      ///< Reset priority (to normal on Windows, to 0 with SCHED_OTHER on linux/unix)
      idle     = THREAD_PRIORITY_IDLE,         ///< Lowest priority (idle threads that just update/monitor periodically)
      veryLow  = THREAD_PRIORITY_LOWEST,       ///< Very low priority (typically used for CPU intensive background tasks)
      lower    = THREAD_PRIORITY_BELOW_NORMAL, ///< Low priority (typically used for background tasks)
      normal   = THREAD_PRIORITY_NORMAL,       ///< Standard priority
      higher   = THREAD_PRIORITY_ABOVE_NORMAL, ///< High priority (typically used for general user input, to increase responsiveness)
      veryHigh = THREAD_PRIORITY_HIGHEST,      ///< Very high priority (warning: - other threads will not get CPU time while this thread is busy;
                                               ///                               - not efficient if too many threads use this priority level;
                                               ///                               - typically used for critical events and user input, to increase responsiveness)
      critical = THREAD_PRIORITY_TIME_CRITICAL ///< Highest priority (only for time critical tasks)
    };
    
    // ---
    
    /// @brief Set scheduler priority for current thread.
    /// @warning On linux/unix systems, the priority cannot exceed the value set by 'ulimit -r'
    inline bool setCurrentThreadPriority(ThreadPriority priority) noexcept {
#     if defined(_WINDOWS) && (defined(_MSC_VER) || !defined(__MINGW32__))
        return (::SetThreadPriority(::GetCurrentThread(), (int)priority) != FALSE);
#     elif defined(__P_USE_POSIX_PTHREAD)
        sched_param args;
        args.sched_priority = (int)priority;
#       ifdef __MINGW32__
          int policy = SCHED_OTHER;
#       else
          int policy = ((int)priority >= (int)ThreadPriority::veryHigh) ? SCHED_FIFO : ((priority != ThreadPriority::reset) ? SCHED_RR : SCHED_OTHER);
#       endif
        return (pthread_setschedparam(pthread_self(), policy, &args) == 0);
#     else
        return false;
#     endif
    }
    
    /// @brief Set scheduler priority for a specific thread.
    /// @warning On linux/unix systems, the priority cannot exceed the value set by 'ulimit -r'
    inline bool setThreadPriority(std::thread& thread, ThreadPriority priority) noexcept {
#     if defined(_WINDOWS) && (defined(_MSC_VER) || !defined(__MINGW32__))
        return (::SetThreadPriority(thread.native_handle(), (int)priority) != FALSE);
#     elif defined(__P_USE_POSIX_PTHREAD)
        sched_param args;
        args.sched_priority = (int)priority;
#       ifdef __MINGW32__
          int policy = SCHED_OTHER;
#       else
          int policy = ((int)priority >= (int)ThreadPriority::veryHigh) ? SCHED_FIFO : ((priority != ThreadPriority::reset) ? SCHED_RR : SCHED_OTHER);
#       endif
        return (pthread_setschedparam(thread.native_handle(), policy, &args) == 0);
#     else
        return false;
#     endif
    }

  }
}
