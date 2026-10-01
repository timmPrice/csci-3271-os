# Build the "util" object library
add_library(util OBJECT 
    list.c 
    vector.c 
    checked_malloc.c)
target_include_directories(util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(util PUBLIC c_std_17)
