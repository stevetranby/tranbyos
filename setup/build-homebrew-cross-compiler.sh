#!/bin/bash

# TODO: rename to bootstrap or setup
banner() {
	echo "|--------------------------------------------|"
	echo "|----- Welcome to the TranbyOS bootstrap ----|"
	echo "|--------------------------------------------|"
}

banner();

osx()
{
	echo "Detected OSX!"
	if [ ! -z "$(which brew)" ]; then
		echo "Homebrew detected! Now updating..."
		brew update
		if [ -z "$(which git)" ]; then
			echo "Now installing git..."
			brew install git
		fi
		if [ "$2" == "qemu" ]; then
			if [ -z "$(which qemu-system-i386)" ]; then
				echo "Installing qemu..."
				brew install qemu
			else
				echo "QEMU already installed!"
			fi
		else
			if [ -z "$(which virtualbox)" ]; then
				echo "Now installing virtualbox..."
				brew cask install virtualbox
			else
				echo "Virtualbox already installed!"
			fi
		fi
	else
		echo "Homebrew does not appear to be installed! Would you like me to install it?"
		printf "(Y/n): "
		read -r installit
		if [ "$installit" == "Y" ]; then
			ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
		else
			echo "Will not install, now exiting..."
			exit
		fi
	fi
	echo "Cloning Redox repo"
	git clone -b "$1" --recursive https://github.com/redox-os/redox.git
	echo "Running Redox setup script..."
	sh redox/setup/osx-homebrew.sh
	echo "Running rust install script"
	sh redox/setup/binary.sh
	endMessage
}

brew update
brew update

# dependencies
brew install git

# clone project
echo "cloning TranbyOS"
git clone https://github.com/stevetranby/tranbyos.git

echo "installing cross-compilers"
# cross-compilers
brew tap homebrew/versions
brew install gcc49
brew tap stevetranby/homebrew-gcc_cross_compilers
brew install nasm
brew install i386-elf-binutils i386-elf-gcc
brew install x86_64-elf-binutils x86_64-elf-gcc
brew install arm-elf-binutils arm-elf-gcc

brew install qemu
brew cask install virtualbox
