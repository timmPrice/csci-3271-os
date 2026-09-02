add_library(queue OBJECT queue.c)
target_compile_features(queue PUBLIC c_std_17)

add_executable(test_queue test_queue.c $<TARGET_OBJECTS:queue>)
target_compile_features(test_queue PUBLIC c_std_17)