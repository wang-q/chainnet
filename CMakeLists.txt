cmake_minimum_required(VERSION 3.26)
project(chainnet C)

set(CMAKE_C_STANDARD 99)

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
