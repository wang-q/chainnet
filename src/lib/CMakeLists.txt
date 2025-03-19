# Create chainnet library
add_library(chainnetlib STATIC
    axt.c
    bits.c
    bPlusTree.c
    boxClump.c
    chain.c
    chainBlock.c
    chainConnect.c
    common.c
    dlist.c
    dnaseq.c
    dnautil.c
    dystring.c
    errAbort.c
    fa.c
    gapCalc.c
    hash.c
    linefile.c
    localmem.c
    net.c
    nib.c
    obscure.c
    options.c
    pipeline.c
    psl.c
    sqlNum.c
    twoBit.c
    udc.c
)

# Set include directories for the library
target_include_directories(chainnetlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compiler options for the library
target_compile_options(chainnetlib PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
