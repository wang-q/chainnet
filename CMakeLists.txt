cmake_minimum_required(VERSION 3.26)
project(chainnet C)

set(CMAKE_C_STANDARD 99)

# Add lib subdirectory
add_subdirectory(src/lib)

add_executable(axtChain
        src/axtChain.c)

# Link against chainnetlib
target_link_libraries(axtChain PRIVATE chainnetlib)
