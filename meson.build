project('mss-nfqws',
        'c',
        version: 'd5b104d7',
        license: 'MIT',
        default_options: [
            'buildtype=plain',
            'c_std=gnu99'
            ]
        )

add_global_arguments(
    [
        '-Wno-logical-op-parentheses',
        '-fno-builtin',
    ],
    language: 'c'
)

# deps
dep_libnetfilter_queue = dependency('libnetfilter_queue')
dep_zlib = dependency('zlib')

nfq_deps = [
    dep_libnetfilter_queue,
    dep_zlib
    ]

# src
crypto_src = [
    'src/crypto/aes-gcm.c',
    'src/crypto/aes.c',
    'src/crypto/gcm.c',
    'src/crypto/hkdf.c',
    'src/crypto/hmac.c',
    'src/crypto/sha224-256.c',
    'src/crypto/usha.c'
    ]

nfq_src = [
    'src/checksum.c',
    'src/conntrack.c',
    'src/darkmagic.c',
    'src/desync.c',
    'src/gzip.c',
    'src/helpers.c',
    'src/hostlist.c',
    'src/nfqws.c',
    'src/protocol.c',
    'src/sec.c',
    'src/strpool.c'
    ]

# set bindir
openwrt = get_option('openwrt')
if openwrt
    bindir = '/bin'
else
    bindir = '/usr/bin'
endif

# build
crypto = static_library('crypto',
                        crypto_src,
                        install: false
                        )

executable('nfqws',
            nfq_src,
            dependencies: nfq_deps,
            link_with: crypto,
            install: true,
            install_dir: bindir
            )

# init script
openwrt = get_option('openwrt')
if openwrt
    install_data(
        'init.d/mss-nfqws',
        install_dir: '/etc/init.d/'
        )
else
    install_data(
        'systemd/mss-nfqws.service',
        install_dir: '/usr/lib/systemd/system/'
    )
    install_data(
        'systemd/nfqws-handler',
        install_dir: '/usr/share/mss-nfqws'
    )
endif

message('OPTS:')
message(' - openwrt: ', openwrt)
message(' - bindir: ', bindir)
