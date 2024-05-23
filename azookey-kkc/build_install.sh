swift build -c release
sudo install -m644 .build/release/libazookey-kkc.so /usr/lib/azookey
sudo install -m644 libazookey_kkc.h /usr/include/azookey
sudo install -m644 azookey.pc /usr/lib/pkgconfig