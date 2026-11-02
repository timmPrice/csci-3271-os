# Builds an object library named "util"
add_subdirectory(util)

# Build the main executable
add_executable(vsfs_test
    test_main.c
    disk.c
    fs.c
    bitmap_block.c
    $<TARGET_OBJECTS:util>)
target_include_directories(vsfs_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(vsfs_test PUBLIC c_std_17)