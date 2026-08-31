add_library(vector OBJECT vector.c)
target_compile_features(vector PUBLIC c_std_17)

add_executable(test_vector test_vector.c $<TARGET_OBJECTS:vector>)
target_compile_features(test_vector PUBLIC c_std_17)