add_library(gtrdatetime datetime.cpp)
add_library(gtr::datetime ALIAS gtrdatetime)
target_include_directories(gtrdatetime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(example main.cpp)
target_link_libraries(example PRIVATE gtr::datetime)
