# Android
if(ANDROID)
    set(android_libs__FOUND ON)
    set(android_libs__LINKED android)
    set(android_libs__INCLUDE 
        ${ANDROID_NDK}/sources/android/native_app_glue 
        ${ANDROID_NDK}/sources
    )
endif()
