set(display_io_libs__FOUND ON)

if(NOT IOS)
    # Android
    if(ANDROID)
      set(display_io_libs__LINKED jnigraphics android)
    
    # MacOS
    elseif(APPLE)
        set(_EXTERNAL_FRAMEWORKS
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

    # Windows
    elseif(WIN32 OR WIN64 OR _WIN32 OR _WIN64 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CWORK_WINDOWS_VERSION AND (CWORK_WINDOWS_VERSION STREQUAL "10" OR CWORK_WINDOWS_VERSION STREQUAL "8"))
            set(display_io_libs__LINKED user32 shcore)
        else()
            set(display_io_libs__LINKED user32)
        endif()   
    
    # Linux
    elseif(CWORK_LINUX_WAYLAND)
        #TODO
    else()
        set(display_io_libs__LINKED X11 ${CMAKE_DL_LIBS})
    endif()
    
    if(CWORK_EXTERN_FRAMEWORKS AND NOT CWORK_EXTERN_FRAMEWORKS_SCOPE)
        if(CWORK_EXTERN_LIBS_SCOPE)
            set(CWORK_EXTERN_FRAMEWORKS_SCOPE ${CWORK_EXTERN_LIBS_SCOPE})
        else()
            set(CWORK_EXTERN_FRAMEWORKS_SCOPE PUBLIC)
        endif()
    endif()
endif()
