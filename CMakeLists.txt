cmake_minimum_required(VERSION 3.5)
project(brandes)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall")

set(SOURCE_FILES src/main.cpp src/graph.cpp src/graph.h
        src/betweenness.h src/betweenness.cpp src/scheduler.h)

add_executable(brandes ${SOURCE_FILES})
target_link_libraries(brandes ${CMAKE_THREAD_LIBS_INIT})

# Doxygen documentation
find_package(Doxygen)
if (DOXYGEN_FOUND)
        configure_file(
                ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
        )
        add_custom_target(
                doc ${DOXYGEN_EXECUTABLE}
                ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
endif (DOXYGEN_FOUND)