# mss-nfqws
mss-nfqws packages the `nfqws` binary from bol-van/zapret[1]. it has the
following differences:
- ported to meson instead of {GNU,BSD}Makefile
- openwrt build recipe with a simplified init file
- saner, simple unit file for systemd

[1] nfqws is licensed under MIT.

## building
builds are done via docker, the sample Dockerfile provided assumes a ramips
mt7621 target. if what you're building for differs, please alter as needed.

```sh
git clone --depth=1 https://github.com/gottaeat/mss-nfqws
cd mss-nfqws/

docker build -t mss-owrt-ramips-mt7621 .

docker run --rm \
    -e MAKEFLAGS="-j$(nproc)"                     \
    -v "${PWD}"/openwrt-feed/:/build/openwrt-feed \
    -v "${PWD}"/bin:/build/bin                    \
    mss-owrt-ramips-mt7621                        \
    bash -c \
        "echo src-link local /build/openwrt-feed >> feeds.conf.default && \
         ./scripts/feeds update packages base local && \
         ./scripts/feeds install mss-nfqws && \
         make defconfig && \
         make package/mss-nfqws/compile"
```

### attn:
if the compilation is noticably slower inside the docker environment, that most
likely is due to the fakeroot process within the container choking up. you might
want to add `--ulimit "nofile=1024:524288"` to your `docker run` args, or change
the line `LimitNOFILE=infinity` to `LimitNOFILE=1024:524288` in your `docker`
and `containerd` systemd service files. for more in-depth reading, please refer
to: https://github.com/moby/moby/issues/38814
