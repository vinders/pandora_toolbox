set(sdl2_static__FOUND ON)

set(SDL_STATIC_ENABLED_BY_DEFAULT ON CACHE STRING "" FORCE)
include(${CMAKE_CURRENT_LIST_DIR}/Findsdl2.cmake)

set(sdl2_static__INCLUDE ${sdl2__INCLUDE})
set(sdl2_static__LINKED ${sdl2__LINKED})
