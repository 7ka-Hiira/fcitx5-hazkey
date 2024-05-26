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

# install pkgconfig file
pkgconfig_dir=$INSTALL_PREFIX/lib/pkgconfig
mkdir -p $pkgconfig_dir
file=$pkgconfig_dir/azookey.pc
echo "prefix=$INSTALL_PREFIX" | tee $file
echo "exec_prefix=\${prefix}" | tee -a $file
echo "libdir=\${prefix}/lib" | tee -a $file
echo "includedir=\${prefix}/include" | tee -a $file
echo "" | tee -a $file
echo "Name: libazookey" | tee -a $file
echo "Description: azooKey Japanese input method library" | tee -a $file
echo "Version: 0.0.1" | tee -a $file
echo "Libs: -L\${libdir}/azookey -lazookey" | tee -a $file
echo "Cflags: -I\${includedir}/azookey" | tee -a $file

install -m644 .build/release/libazookey-kkc.so $INSTALL_PREFIX/lib/azookey
install -m644 libazookey_kkc.h $INSTALL_PREFIX/include/azookey