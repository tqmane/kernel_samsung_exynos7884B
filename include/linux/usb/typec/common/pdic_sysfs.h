/*
 *
 * Copyright (C) 2017-2019 Samsung Electronics
 *
 * Author:Wookwang Lee. <wookwang.lee@samsung.com>,
 * Author:Guneet Singh Khurana  <gs.khurana@samsung.com>,
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __PDIC_SYSFS__
#define __PDIC_SYSFS__

extern const struct attribute_group pdic_sysfs_group;

enum {
	BUILT_IN = 0,
	UMS = 1,
	SPU = 2,
};

enum pdic_sysfs_property {
	PDIC_SYSFS_PROP_CHIP_NAME = 0,
	PDIC_SYSFS_PROP_CUR_VERSION,
	PDIC_SYSFS_PROP_SRC_VERSION,
	PDIC_SYSFS_PROP_LPM_MODE,
	PDIC_SYSFS_PROP_STATE,
	PDIC_SYSFS_PROP_RID,
	PDIC_SYSFS_PROP_CTRL_OPTION,
	PDIC_SYSFS_PROP_BOOTING_DRY,
	PDIC_SYSFS_PROP_FW_UPDATE,
	PDIC_SYSFS_PROP_FW_UPDATE_STATUS,
	PDIC_SYSFS_PROP_FW_WATER,
	PDIC_SYSFS_PROP_DEX_FAN_UVDM,
	PDIC_SYSFS_PROP_ACC_DEVICE_VERSION,
	PDIC_SYSFS_PROP_DEBUG_OPCODE,
	PDIC_SYSFS_PROP_CONTROL_GPIO,
	PDIC_SYSFS_PROP_USBPD_IDS,
	PDIC_SYSFS_PROP_USBPD_TYPE,	/* for SWITCH_STATE */
	PDIC_SYSFS_PROP_PD_PIN_STATUS,
	PDIC_SYSFS_PROP_RAM_TEST,
	PDIC_SYSFS_PROP_SBU_ADC,	
	PDIC_SYSFS_PROP_SET_WATER_THRESHOLD,
#if defined(CONFIG_S2MU106_TYPEC_WATER)
	PDIC_SYSFS_PROP_USBPD_WATER_CHECK,
#endif
	PDIC_SYSFS_PROP_VSAFE0V_STATUS,
	PDIC_SYSFS_PROP_MAX_COUNT,
};
struct _pdic_data_t;
typedef struct _pdic_sysfs_property_t {
	enum pdic_sysfs_property *properties;
	size_t num_properties;
	int (*get_property)(struct _pdic_data_t *ppdic_data,
			     enum pdic_sysfs_property prop,
			     char *buf);
	ssize_t (*set_property)(struct _pdic_data_t *ppdic_data,
			     enum pdic_sysfs_property prop,
			     const char *buf,
				 size_t size);
	/* Decides whether userspace can change a specific property */
	int (*property_is_writeable)(struct _pdic_data_t *ppdic_data,
				      enum pdic_sysfs_property prop);
} pdic_sysfs_property_t, *ppdic_sysfs_property_t;

void pdic_sysfs_init_attrs(void);
#endif

