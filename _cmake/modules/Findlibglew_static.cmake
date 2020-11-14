if(NOT IOS AND NOT APPLE AND NOT ANDROID)
    set(libglew_static__FOUND ON)

    set(GLEW_STATIC ON CACHE STRING "" FORCE)
    include(${CMAKE_CURRENT_LIST_DIR}/Findlibglew.cmake)
    
    set(libglew_static__INCLUDE ${libglew__INCLUDE})
    set(libglew_static__LINKED ${libglew__LINKED})
endif()