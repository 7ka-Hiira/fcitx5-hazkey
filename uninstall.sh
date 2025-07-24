#!/usr/bin/env sh
set -e

# set default install prefix if not set
if [ -z "$INSTALL_PREFIX" ]; then
    echo "INSTALL_PREFIX is not set. Using /usr as default."
    export INSTALL_PREFIX="/usr"
fi

rm -rf $INSTALL_PREFIX/share/hazkey
rm -rf $INSTALL_PREFIX/include/hazkey
rm -f $INSTALL_PREFIX/lib/hazkey/hazkey_server
rm -f $INSTALL_PREFIX/lib/pkgconfig/hazkey.pc
rm -f $INSTALL_PREFIX/lib/fcitx5/fcitx5-hazkey.so
rm -f $INSTALL_PREFIX/share/fcitx5/addon/hazkey.conf
rm -f $INSTALL_PREFIX/share/fcitx5/inputmethod/hazkey.conf
rm -f $INSTALL_PREFIX/share/metainfo/org.fcitx.Fcitx5.Addon.Hazkey.metainfo.xml
rm -f $INSTALL_PREFIX/share/locale/*/LC_MESSAGES/fcitx5-hazkey.mo

echo "Successfully uninstalled fcitx5-hazkey."
