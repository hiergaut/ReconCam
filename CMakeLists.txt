cmake_minimum_required(VERSION 2.8.12)
project( ReconCam )

find_package( OpenCV REQUIRED )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories( ${OpenCV_INCLUDE_DIRS} )

#file(GLOB SOURCES *.cpp *.hpp)
set(SOURCES main.cpp Gpio.hpp System.hpp utils.hpp)
add_executable( ${PROJECT_NAME} ${SOURCES})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Threads REQUIRED)
#target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})

add_compile_definitions(PC)
#add_compile_definitions(DETECTION)

add_definitions(-DPROJECT_DIR=\"${CMAKE_SOURCE_DIR}/\")


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/gpio
    ${CMAKE_CURRENT_BINARY_DIR}/gpio
    )

