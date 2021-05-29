set(opengles_libs__FOUND ON)

if(IOS)
    find_library(OPENGLES_FRAMEWORKS OpenGLES)
    set(opengles_libs__INCLUDE ${OPENGL_INCLUDE_DIR})
    
    if(CWORK_EXTERN_FRAMEWORKS)
        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} OpenGLES)
    else()
        set(CWORK_EXTERN_FRAMEWORKS OpenGLES)
    endif()
    
elseif(ANDROID)
    if(ANDROID_STANDALONE_TOOLCHAIN)
        set(opengles_libs__INCLUDE "${ANDROID_STANDALONE_TOOLCHAIN}/usr/include")
    endif()
    
    set(opengles_libs__LINKED android EGL GLESv3)
    
elseif(CWORK_LINUX_WAYLAND)
    #...

else()
    #...
endif()

if(CWORK_EXTERN_FRAMEWORKS AND NOT CWORK_EXTERN_FRAMEWORKS_SCOPE)
    if(CWORK_EXTERN_LIBS_SCOPE)
        set(CWORK_EXTERN_FRAMEWORKS_SCOPE ${CWORK_EXTERN_LIBS_SCOPE})
    else()
        set(CWORK_EXTERN_FRAMEWORKS_SCOPE PUBLIC)
    endif()
endif()
