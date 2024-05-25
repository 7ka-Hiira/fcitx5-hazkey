#!/usr/bin/env bash

# this script is called from 'build_install.sh'

# check lib and include directory exists
if [ ! -d "$INSTALL_PREFIX/lib" ] || [ ! -d "$INSTALL_PREFIX/include" ]; then
    echo "INSTALL_PREFIX is not set correctly."
    exit 1
fi

# install azookey libs
mkdir -p $INSTALL_PREFIX/lib/azookey
mkdir -p $INSTALL_PREFIX/include/azookey

install -m644 .build/release/libazookey-kkc.so $INSTALL_PREFIX/lib/azookey
install -m644 libazookey_kkc.h $INSTALL_PREFIX/include/azookey
install -m644 azookey.pc $INSTALL_PREFIX/lib/pkgconfig