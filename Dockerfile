FROM debian:12.11 AS nfqws-build

ENV FORCE_UNSAFE_CONFIGURE=1

WORKDIR /build

RUN \
    apt update && \
    apt install -y \
        bison build-essential clang curl file flex g++ gawk gettext git \
        libncurses-dev libssl-dev python3-dev python3-distutils \
        python3-setuptools rsync swig unzip wget zlib1g-dev zstd
