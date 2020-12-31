cmake_minimum_required(VERSION 3.16)

project(stitch)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}
                    "${PROJECT_SOURCE_DIR}/image_stitch")

add_subdirectory(image_stitch)

add_library(video_tools video_sync.cpp)
target_link_libraries(video_tools ${OpenCV_LIBS})

add_executable(sync_videos sync_videos.cpp)
target_link_libraries(sync_videos ${OpenCV_LIBS} video_tools stdc++fs)

add_executable(stitch_videos stitch_videos.cpp)
target_link_libraries(stitch_videos ${OpenCV_LIBS} image_stitch video_tools stdc++fs)
