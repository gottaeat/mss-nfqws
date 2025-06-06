# mss-nfqws
mss-nfqws packages the `nfqws` binary from bol-van/zapret[1]. it has the
following differences:
- ported to meson instead of {GNU,BSD}Makefile
- openwrt build recipe with a simplified init file
- saner, simple unit file for systemd

[1] nfqws is licensed under MIT.

## building
builds are done via docker. the docker compose file provided assumes you're
building for a ramips mt7621 target on arm64.

to build your own, get the SDK for your target, put it in `./files` and update
the platform definition and the SDK tarball name in the compose example, then do
`docker cmopose up`.

resulting package can be found at `./out` afterwards.

__WARNING__: if the compilation is noticably slower, that could be due to the
fakeroot process that openwrt sdk is using choking up. so keep the `ulimit`
definition in the example docker compose file OR set `LimitNOFILE` to
`1024:524288` in your docker and containerd systemd service files. for more info
refer to: https://github.com/moby/moby/issues/38814
