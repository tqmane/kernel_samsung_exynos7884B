#
# Copyright (c) 2013 Samsung Electronics Co., Ltd.
#		http://www.samsung.com
#
# Licensed under GPLv2
#

subdir-ccflags-$(CONFIG_EXYNOS_SUPPORT_DOZE)	+= -DCONFIG_EXYNOS_DOZE
subdir-ccflags-y	+= -DCONFIG_EXYNOS_DPU20

obj-$(CONFIG_EXYNOS_DPP) += dpp.o
dpp-y := dpp_drv.o
obj-$(CONFIG_EXYNOS_MIPI_DSIM) += dsim.o
dsim-y += dsim_drv.o
obj-$(CONFIG_SOC_EXYNOS7885) += dsim_reg.o dpp_reg.o decon_reg.o
obj-$(CONFIG_EXYNOS_DECON_7885) += decon.o
obj-$(CONFIG_DECON_EVENT_LOG) += event_log.o
obj-$(CONFIG_EXYNOS_SUPPORT_DOZE) += decon_doze.o dsim_doze.o
decon-y += decon_core.o decon_dsi.o  helper.o win_update.o fence.o
obj-$(CONFIG_EXYNOS_BTS) += bts.o
obj-y += panels/
