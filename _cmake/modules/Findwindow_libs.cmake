set(window_libs__FOUND ON)

if(NOT IOS AND NOT ANDROID)
    # MacOS
    if(APPLE)
        set(_EXTERNAL_FRAMEWORKS
            Cocoa
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
        set(window_libs__LINKED kernel32 user32 comdlg32 ole32 oleaut32 gdi32)
        if(_DRAG_DROP_SUPPORT)
            set(window_libs__LINKED ${window_libs__LINKED} shell32)
        endif()
        
    # Linux
    elseif(CWORK_LINUX_WAYLAND)
        message("-- checking wayland packages... --")
        message("note: libwayland-dev, wayland-protocols, libxkbcommon-dev must be installed")
        include(GNUInstallDirs)
        include(FindPkgConfig)
        pkg_check_modules(Wayland REQUIRED
            wayland-client>=0.2.7
            wayland-cursor>=0.2.7
            xkbcommon
        )
        include(CheckIncludeFiles)
        include(CheckFunctionExists)
        check_include_files(xkbcommon/xkbcommon-compose.h _P_ENABLE_LINUX_XKBCOMMON_COMPOSE)
        check_function_exists(memfd_create _P_ENABLE_LINUX_MEMFD_CREATE)
        
        set(_LINUX_WAYLAND_LINKED ON)
        set(window_libs__INCLUDE ${Wayland_INCLUDE_DIRS})
        set(window_libs__LINKED ${Wayland_LINK_LIBRARIES})
    else()
        set(window_libs__LINKED X11)
    endif()

    if(CWORK_EXTERN_FRAMEWORKS AND NOT CWORK_EXTERN_FRAMEWORKS_SCOPE)
        if(CWORK_EXTERN_LIBS_SCOPE)
            set(CWORK_EXTERN_FRAMEWORKS_SCOPE ${CWORK_EXTERN_LIBS_SCOPE})
        else()
            set(CWORK_EXTERN_FRAMEWORKS_SCOPE PUBLIC)
        endif()
    endif()
endif()
