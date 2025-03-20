cmake_minimum_required(VERSION 3.15)
project(chainnet C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set compiler options
add_compile_options(
    -Wall
    -Wextra
    -Wno-sign-compare
    -Wno-visibility
    -Wno-unused-function
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-deprecated-non-prototype
)

# Add debug information
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

# Add lib subdirectory
add_subdirectory(src/lib)

# Define executable list
set(EXECUTABLES
    axtChain
    axtSort
    axtToMaf
    chainAntiRepeat
    chainMergeSort
    chainNet
    chainPreNet
    chainSplit
    chainStitchId
    faToTwoBit
    lavToPsl
    netChainSubset
    netFilter
    netSplit
    netSyntenic
    netToAxt
)

# Add executables and link against chainnetlib
foreach(executable ${EXECUTABLES})
    add_executable(${executable} src/${executable}.c)
    target_link_libraries(${executable} PRIVATE chainnetlib)
endforeach()

# Set installation rules
install(TARGETS ${EXECUTABLES}
    RUNTIME DESTINATION bin
)

# Export compile commands (for code analysis tools)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
