#
# Makefile for the Marvell Gigabit Ethernet driver
#
ifeq ($(CONFIG_ARCH_FEROCEON_KW2),y)
	include $(CURDIR)/arch/arm/mach-feroceon-kw2/config/mvRules.mk
endif

obj-$(CONFIG_MV_ETH_BM) += bm_sysfs.o
obj-$(CONFIG_MV_ETH_BM) += mv_eth_bm.o
