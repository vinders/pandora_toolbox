set(time_libs__FOUND ON)

if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
    set(time_libs__LINKED winmm)
endif()
