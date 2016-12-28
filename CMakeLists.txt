cmake_minimum_required(VERSION 3.6)
project(brandes)
find_package ( Threads REQUIRED )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -Wall")



# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} ${CMAKE_THREAD_LIBS_INIT} )
    endif()
endmacro()

set(SOURCE_FILES src/main.cpp src/betweenness.h src/betweenness.cpp src/graph.cpp src/graph.h src/scheduler.h)
add_executable(brandes ${SOURCE_FILES})
