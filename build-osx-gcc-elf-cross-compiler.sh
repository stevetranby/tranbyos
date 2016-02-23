## TODO: create script to follow this wiki page
## Note: download prebuilt grub img and use hdutils instead of grub-mkrescue

brew install binutils
brew install gcc gpp
brew install bison flex gmp mpfr mpc isl cloog
brew install libiconv
brew install texinfo

# This is only necessary for MacOS users.
export CC=/usr/local/bin/gcc-4.8
export CXX=/usr/local/bin/g++-4.8
export CPP=/usr/local/bin/cpp-4.8
export LD=/usr/local/bin/gcc-4.8

# Add to bash_profile (or similar)
TOOLCHAIN=""
export PREFIX="$TOOLCHAIN/opt/cross"
export PREFIX="$TOOLCHAIN/local/cross-elf"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"


# Build cross-Binutils
cd $HOME/src
mkdir build-binutils
cd build-binutils
../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror
make
make install


# Build cross-GCC

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
