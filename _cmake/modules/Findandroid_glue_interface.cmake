# Android
if(ANDROID)
    set(android_glue_interface__FOUND ON)
    
    if(ANDROID_NDK)
        set(_PATH_PREFIX ${ANDROID_NDK})
    elseif(ANDROID_NDK_HOME)
        set(_PATH_PREFIX ${ANDROID_NDK_HOME})
    elseif(DEFINED ENV{ANDROID_NDK_HOME})
        set(_PATH_PREFIX $ENV{ANDROID_NDK_HOME})
    else()
        set(_PATH_PREFIX /usr/local/android-ndk)
    endif()

    set(android_glue_interface__INCLUDE ${_PATH_PREFIX}/sources/android/native_app_glue)
    unset(_PATH_PREFIX)
endif()
