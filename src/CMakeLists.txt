cmake_minimum_required(VERSION 3.16)

project(stitch)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR})
add_executable(sync_videos sync_videos.cpp video_sync.cpp)
add_executable(stitch_videos stitch_videos.cpp video_sync.cpp stitch_compute.cpp image_struct.cpp)
target_link_libraries(sync_videos ${OpenCV_LIBS} stdc++fs)
target_link_libraries(stitch_videos ${OpenCV_LIBS} stdc++fs)
