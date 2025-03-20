# UCSC chain/net pipeline

kent-core-479

```bash
ASM="zig cc" \
CC="zig cc" \
CXX="zig c++" \
CFLAGS="-I$HOME/.cbp/include" \
LDFLAGS="-L$HOME/.cbp/lib" \
cmake \
    -S . -B build &&
    cmake --build build

```


```bash
CC="zig cc" \
CXX="zig c++" \
AR="zig ar" \
RANLIB="zig ranlib" \
CFLAGS="-O3 -fPIC" \
CXXFLAGS="-O3 -fPIC -felide-constructors -fno-exceptions -fno-rtti" \
ABI_CHECK="" \
    ./configure \
    --prefix=$HOME/collect \
    --disable-dependency-tracking \
    --disable-shared \
    --enable-static \
    --without-docs \
    --without-man \
    --without-debug \
    --without-server

```
