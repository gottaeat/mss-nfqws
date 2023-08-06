#!/bin/sh
_FEED_PATH="${PWD}"/openwrt-feed

mkdir build/
cd    build/

curl -L -O https://archive.openwrt.org/releases/22.03.5/targets/ramips/mt7621\
/openwrt-sdk-22.03.5-ramips-mt7621_gcc-11.2.0_musl.Linux-x86_64.tar.xz

tar xf openwrt-sdk-22.03.5-ramips-mt7621_gcc-11.2.0_musl.Linux-x86_64.tar.xz
cd     openwrt-sdk-22.03.5-ramips-mt7621_gcc-11.2.0_musl.Linux-x86_64/

echo "src-link local ${_FEED_PATH}" >> feeds.conf.default
./scripts/feeds update packages base local
./scripts/feeds install mss-nfqws

make defconfig
make package/mss-nfqws/compile

mv bin/packages/mipsel_24kc/local/mss-nfqws*.ipk ../../
cd ../../; rm -rf build/
