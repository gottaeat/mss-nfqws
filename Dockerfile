FROM archlinux:latest AS owrt-build

RUN \
    pacman-key --init && \
    pacman-key --populate archlinux && \
    pacman -Syu --noconfirm \
        asciidoc autoconf automake base-devel bash binutils bison bzip2 \
        fakeroot file findutils flex gawk gcc gettext git grep groff gzip \
        help2man intltool libelf libtool libxslt m4 make ncurses openssl \
        patch perl-extutils-makemaker pkgconf python rsync sed swig texinfo \
        time unzip util-linux wget which zlib

RUN \
    wget https://downloads.openwrt.org/releases/23.05.0/targets/ramips/mt7621/openwrt-sdk-23.05.0-ramips-mt7621_gcc-12.3.0_musl.Linux-x86_64.tar.xz && \
    tar xf openwrt-sdk-23.05.0-ramips-mt7621_gcc-12.3.0_musl.Linux-x86_64.tar.xz && \
    mv openwrt-sdk-23.05.0-ramips-mt7621_gcc-12.3.0_musl.Linux-x86_64/ /build

WORKDIR /build
CMD ["/bin/bash"]
