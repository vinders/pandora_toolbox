if(NOT IOS AND NOT ANDROID)
    set(glfw3_static__FOUND ON)

    set(GLFW_STATIC ON CACHE BOOL "GLFW_STATIC" FORCE)
    set(GLFW3_STATIC ON CACHE STRING "" FORCE)
    include(${CMAKE_CURRENT_LIST_DIR}/Findglfw3.cmake)
    
    set(glfw3_static__LINKED ${glfw3__LINKED})
endif()
