# Build the "util" object library
add_library(util OBJECT
    list.c
    vector.c
    checked_malloc.c
    random.c
    string_util.c
    min_max.c)
target_include_directories(util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(util PUBLIC c_std_17)
