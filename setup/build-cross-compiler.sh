# This is a work in progress
# Build a cross-compiler for i686

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Build binutils
cd $HOME/src
mkdir build-binutils
cd build-binutils
../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror
make
make install


# Build GCC

cd $HOME/src

# If you wish to build these packages as part of gcc:
mv libiconv-x.y.z gcc-x.y.z/libiconv # Mac OS X users
mv gmp-x.y.z gcc-x.y.z/gmp
mv mpfr-x.y.z gcc-x.y.z/mpfr
mv mpc-x.y.z gcc-x.y.z/mpc

mkdir build-gcc
cd build-gcc
../gcc-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc


# Test Version

$HOME/opt/cross/bin/$TARGET-gcc --version

# Add cross compiler to path
export PATH="$HOME/opt/cross/bin:$PATH"
