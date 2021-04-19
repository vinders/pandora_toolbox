set(android_libs__FOUND ON)

# Android
if(ANDROID)
    set(android_libs__INCLUDE 
        ${ANDROID_NDK}/sources/android/native_app_glue 
        ${ANDROID_NDK}/sources
    )
endif()
