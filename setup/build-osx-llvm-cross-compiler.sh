
# This is a work in progress
# Build an LLVM cross-compiler on OSX

## Build Metta (OS) and dev from there??
# https://github.com/berkus/metta
# https://github.com/berkus/metta/blob/develop/build_toolchain.sh


# base
which git || (echo "Install git: brew install git"; exit)
which cmake || (echo "Install cmake: brew install cmake"; exit)
which ninja || (echo "Install ninja: brew install ninja"; exit)

# deps
brew install yasm boost ossp-uuid openssl cdrtools cmake ninja 

# bochs
brew cask install --force Caskroom/cask/xquartz
brew install homebrew/x11/bochs


# setup and build toolchain
mkdir -p toolchain/{build/{llvm},sources}
cd toolchain/
