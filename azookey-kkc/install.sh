#!/usr/bin/env bash

# this script is called from 'build_install.sh'

# check lib and include directory exists
if [ ! -d "$INSTALL_PREFIX/lib" ] || [ ! -d "$INSTALL_PREFIX/include" ]; then
    echo "INSTALL_PREFIX is not set correctly."
    exit 1
fi

# install hazukey libs
mkdir -p $INSTALL_PREFIX/lib/hazukey
mkdir -p $INSTALL_PREFIX/include/hazukey

# install pkgconfig file
pkgconfig_dir=$INSTALL_PREFIX/lib/pkgconfig
mkdir -p $pkgconfig_dir
file=$pkgconfig_dir/hazukey.pc
echo "prefix=$INSTALL_PREFIX" | tee $file
echo "exec_prefix=\${prefix}" | tee -a $file
echo "libdir=\${prefix}/lib" | tee -a $file
echo "includedir=\${prefix}/include" | tee -a $file
echo "" | tee -a $file
echo "Name: libhazukey" | tee -a $file
echo "Description: Hazukey Japanese input method library" | tee -a $file
echo "Version: 0.0.1" | tee -a $file
echo "Libs: -L\${libdir}/hazukey -lhazukey" | tee -a $file
echo "Cflags: -I\${includedir}/hazukey" | tee -a $file

install -m644 .build/release/libhazukey.so $INSTALL_PREFIX/lib/hazukey
install -m644 libhazukey.h $INSTALL_PREFIX/include/hazukey