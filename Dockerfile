FROM alpine:3.18.2 AS owrt-build

ENV LANG=en_US.utf8

RUN \
    apk --no-cache add \
        argp-standalone asciidoc bash bc binutils bzip2 cdrkit coreutils \
        diffutils elfutils-dev findutils flex g++ gawk gcc gettext git grep \
        gzip intltool libxslt linux-headers make musl-fts-dev musl-libintl \
        musl-obstack-dev ncurses-dev openssl-dev patch perl python3-dev rsync \
        tar unzip util-linux wget zlib-dev

RUN \
    wget https://archive.openwrt.org/releases/22.03.5/targets/ramips/mt7621/openwrt-sdk-22.03.5-ramips-mt7621_gcc-11.2.0_musl.Linux-x86_64.tar.xz && \
    tar xf openwrt-sdk-22.03.5-ramips-mt7621_gcc-11.2.0_musl.Linux-x86_64.tar.xz && \
    mv openwrt-sdk-22.03.5-ramips-mt7621_gcc-11.2.0_musl.Linux-x86_64/ /build

WORKDIR /build
CMD ["/bin/bash"]
