add_library(image_stitch image_struct.cpp stitch_compute.cpp)
target_link_libraries(image_stitch ${OpenCV_LIBS})
