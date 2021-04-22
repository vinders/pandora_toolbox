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
    
    include(${CMAKE_CURRENT_LIST_DIR}/path_utils.cmake)

    cwork_unknown_path_to_absolute(${_PATH_PREFIX} "/sources/android/native_app_glue/android_native_app_glue.c")
    set(android_glue__INCLUDE ${CWORK_VALID_ABSOLUTE_PATH}/sources/android/native_app_glue)
    set(android_glue__LINKED android)
    set(android_glue__SOURCE ${CWORK_VALID_ABSOLUTE_PATH}/sources/android/native_app_glue/android_native_app_glue.c)
    unset(CWORK_VALID_ABSOLUTE_PATH)
    unset(_PATH_PREFIX)
endif()
