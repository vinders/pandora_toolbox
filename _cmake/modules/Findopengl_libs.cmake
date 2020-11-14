set(opengl_libs__FOUND ON)

if(IOS)
    find_library(OPENGLES_FRAMEWORKS OpenGLES)
    set(opengl_libs__INCLUDE ${OPENGL_INCLUDE_DIR})
    
    if(CWORK_EXTERN_FRAMEWORKS)
        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} OpenGLES)
    else()
        set(CWORK_EXTERN_FRAMEWORKS OpenGLES)
    endif()
    
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    set(opengl_libs__INCLUDE ${OPENGL_INCLUDE_DIR})
    
    set(_EXTERNAL_FRAMEWORKS
        OpenGL
        GLUT
        Cocoa
        IOKit
        CoreFoundation
    )
    if(CWORK_EXTERN_FRAMEWORKS)
        set(CWORK_EXTERN_FRAMEWORKS ${CWORK_EXTERN_FRAMEWORKS} ${_EXTERNAL_FRAMEWORKS})
    else()
        set(CWORK_EXTERN_FRAMEWORKS ${_EXTERNAL_FRAMEWORKS})
    endif()
    unset(_EXTERNAL_FRAMEWORKS)
    
    set(opengl_libs__LINKED ${OPENGL_LIBRARIES})
    
elseif(ANDROID)
    if(ANDROID_STANDALONE_TOOLCHAIN)
        set(opengl_libs__INCLUDE "${ANDROID_STANDALONE_TOOLCHAIN}/usr/include")
    endif()
    
    set(opengl_libs__LINKED android EGL GLESv3)
    
else()
    find_package(OpenGL REQUIRED)
    if(WIN32 OR WIN64 OR _WIN32 OR _WIN64)
        set(opengl_libs__LINKED ${OPENGL_LIBRARIES})
    else()
        set(opengl_libs__LINKED X11 ${OPENGL_LIBRARIES})
    endif()
endif()

if(CWORK_EXTERN_FRAMEWORKS AND NOT CWORK_EXTERN_FRAMEWORKS_SCOPE)
    if(CWORK_EXTERN_LIBS_SCOPE)
        set(CWORK_EXTERN_FRAMEWORKS_SCOPE ${CWORK_EXTERN_LIBS_SCOPE})
    else()
        set(CWORK_EXTERN_FRAMEWORKS_SCOPE PUBLIC)
    endif()
endif()
