# Android
if(ANDROID)
    set(android_glue__FOUND ON)
    
    if(ANDROID_NDK)
        set(_PATH_PREFIX ${ANDROID_NDK})
    elseif(ANDROID_NDK_HOME)
        set(_PATH_PREFIX ${ANDROID_NDK_HOME})
    elseif(DEFINED ENV{ANDROID_NDK_HOME})
        set(_PATH_PREFIX $ENV{ANDROID_NDK_HOME})
    else()
        set(_PATH_PREFIX /usr/local/android-ndk)
    endif()

    set(android_glue__INCLUDE ${_PATH_PREFIX}/sources/android/native_app_glue)
    set(android_glue__SOURCE ${_PATH_PREFIX}/sources/android/native_app_glue/android_native_app_glue.c)
    unset(_PATH_PREFIX)
endif()
