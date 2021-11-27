
cmake_minimum_required(VERSION 2.8)
project( ReconCam )

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( ${PROJECT_NAME} main.cpp function.cpp Identity.cpp )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )

