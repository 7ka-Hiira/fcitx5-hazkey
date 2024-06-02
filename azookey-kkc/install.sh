#!/usr/bin/env bash

# this script is called from 'build_install.sh'

# check lib and include directory exists
if [ ! -d "$INSTALL_PREFIX/lib" ] || [ ! -d "$INSTALL_PREFIX/include" ]; then
    echo "INSTALL_PREFIX is not set correctly."
    exit 1
fi

# install hazkey libs
mkdir -p $INSTALL_PREFIX/lib/hazkey
mkdir -p $INSTALL_PREFIX/include/hazkey

# install pkgconfig file
pkgconfig_dir=$INSTALL_PREFIX/lib/pkgconfig
mkdir -p $pkgconfig_dir
file=$pkgconfig_dir/hazkey.pc

echo ""
echo "**********"
echo "prefix=$INSTALL_PREFIX" | tee $file
echo "exec_prefix=\${prefix}" | tee -a $file
echo "libdir=\${prefix}/lib" | tee -a $file
echo "includedir=\${prefix}/include" | tee -a $file
echo "" | tee -a $file
echo "Name: libhazkey" | tee -a $file
echo "Description: Hazkey Japanese input method library" | tee -a $file
echo "Version: 0.0.1" | tee -a $file
echo "Libs: -L\${libdir}/hazkey -lhazkey" | tee -a $file
echo "Cflags: -I\${includedir}/hazkey" | tee -a $file
echo "**********"
echo ""
echo "pkgconfig file installed at $file"
echo ""

install -m644 .build/release/libhazkey.so $INSTALL_PREFIX/lib/hazkey
install -m644 libhazkey.h $INSTALL_PREFIX/include/hazkey