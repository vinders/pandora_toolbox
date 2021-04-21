# Android
if(ANDROID)
    set(android_glue__FOUND ON)
    set(android_glue__INCLUDE 
        ${ANDROID_NDK}/sources/android/native_app_glue 
        ${ANDROID_NDK}/sources
    )
    set(android_glue__SOURCE 
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
endif()
