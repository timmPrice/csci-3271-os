# Builds an object library named "util"
add_subdirectory(util)

# Build the pager objects as a library
add_library(pager OBJECT
    pager.c
    dummy_pager.c
    fifo_pager.c
    lru_pager.c
    clock_pager.c
    random_pager.c
    opt_pager.c
    $<TARGET_OBJECTS:util>)
target_include_directories(pager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(pager PUBLIC c_std_17)

# Build the main executable, which depends on util and the pager objects
add_executable(paging_simulation
    main.c
    $<TARGET_OBJECTS:util>
    $<TARGET_OBJECTS:pager>)
target_include_directories(paging_simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(paging_simulation PUBLIC c_std_17)
