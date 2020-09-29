set(thread__FOUND ON)

find_package(Threads)
if(CMAKE_THREAD_LIBS_INIT)
    set(thread__LINKED ${CMAKE_THREAD_LIBS_INIT})
endif()
