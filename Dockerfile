FROM archlinux:latest AS owrt-build

RUN \
    pacman-key --init && \
    pacman-key --populate archlinux && \
    pacman -Syu --noconfirm \
        asciidoc autoconf automake base-devel bash binutils bison bzip2 \
        fakeroot file findutils flex gawk gcc gettext git grep groff gzip \
        help2man intltool libelf libtool libxslt m4 make ncurses openssl \
        patch perl-extutils-makemaker pkgconf python rsync sed swig texinfo \
        time unzip util-linux wget which zlib python-distutils-extra

RUN \
    wget https://downloads.openwrt.org/releases/24.10.0/targets/lantiq/xrx200/openwrt-sdk-24.10.0-lantiq-xrx200_gcc-13.3.0_musl.Linux-x86_64.tar.zst && \
    tar xf openwrt-sdk-24.10.0-lantiq-xrx200_gcc-13.3.0_musl.Linux-x86_64.tar.zst && \
    mv openwrt-sdk-24.10.0-lantiq-xrx200_gcc-13.3.0_musl.Linux-x86_64/ /build

WORKDIR /build
CMD ["/bin/bash"]
