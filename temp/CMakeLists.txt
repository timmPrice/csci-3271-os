cmake_minimum_required(VERSION 3.22)
project(vsfs
    LANGUAGES C)

# Disable Microsoft's warnings about strcpy, strlen, and other functions
# The "recommended" replacements are buggy and non-portable (see https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1967.htm)
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# All source code is in the "src" subdirectory,
# so everything else is in that directory's CMakeLists
add_subdirectory(src)
