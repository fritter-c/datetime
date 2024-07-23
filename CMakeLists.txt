add_library(gtrdatetime datetime.cpp)
add_library(gtr::datetime ALIAS gtrdatetime)
target_compile_options(gtrdatetime PRIVATE --save-temps)
target_include_directories(gtrdatetime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
