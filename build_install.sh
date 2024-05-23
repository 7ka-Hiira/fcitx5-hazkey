#!/usr/bin/env sh
set -e

# check if azookey-kkc and fcitx5 directory exists
if [ ! -d "azookey-kkc" ] || [ ! -d "fcitx5" ]; then
    echo "This script must be run from the root of this repository."
    exit 1
fi

# build azookey library
cd azookey-kkc
swift build -c release

# set install prefix if not set
if [ -z "$INSTALL_PREFIX" ]; then
    printf "INSTALL_PREFIX is not set. Do you want to install to /usr? [y/N] "
    read -r response
    if [ "$response" = "y" ] || [ "$response" = "Y" ]; then
        export INSTALL_PREFIX="/usr"
    else
        echo "Please set INSTALL_PREFIX to the directory where you want to install azookey."
        exit 1
    fi
fi

# install azookey libs
chmod +x ./install.sh
sudo -E ./install.sh

# build fcitx5 module
cd ../fcitx5
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX ..
make -j$(nproc)

# install fcitx5 module
sudo make install