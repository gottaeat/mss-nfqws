include $(TOPDIR)/rules.mk

PKG_NAME:=mss-nfqws
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/gottaeat/mss-nfqws.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=ec453900a1dc6c4ffc35fea6d06844e83b2113c4

PKG_BUILD_DEPENDS:=libcap

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/mss-nfqws
	SECTION:=net
	CATEGORY:=Network
	TITLE:=mss-nfqws
	DEPENDS:=+libnetfilter-queue +zlib +nftables +kmod-nft-queue
	URL:=https://github.com:gottaeat/mss-nfqws
endef

MESON_ARGS += -Dopenwrt=true

define Package/mss-nfqws/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/init.d/* $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,mss-nfqws))
