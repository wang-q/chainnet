# Create chainnet library
add_library(chainnetlib STATIC
    aliType.c
    axt.c
    binRange.c
    bits.c
    bPlusTree.c
    chain.c
    chainBlock.c
    chainConnect.c
    chainNet.c
    chainToAxt.c
    chainToPsl.c
    common.c
    dlist.c
    dnaseq.c
    dnautil.c
    dystring.c
    errAbort.c
    fa.c
    filePath.c
    ffAli.c
    gapCalc.c
    hash.c
    lav.c
    linefile.c
    localmem.c
    maf.c
    memalloc.c
    nib.c
    nibTwo.c
    obscure.c
    options.c
    osunix.c
    pipeline.c
    portimpl.c
    psl.c
    quickHeap.c
    rangeTree.c
    rbTree.c
    sqlList.c
    sqlNum.c
    twoBit.c
    udc.c
    verbose.c
    wildcmp.c
)

# Set include directories for the library
target_include_directories(chainnetlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compiler options for the library
target_compile_options(chainnetlib PRIVATE
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

# Link with math library
target_link_libraries(chainnetlib PUBLIC m)
