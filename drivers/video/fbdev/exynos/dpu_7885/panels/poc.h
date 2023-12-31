/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __PANEL_POC_H__
#define __PANEL_POC_H__

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/notifier.h>

#define POC_FA7_TOTAL_SIZE	(302686)
#define POC_IMG_SIZE	POC_FA7_TOTAL_SIZE
#define POC_IMG_ADDR	(0x0)
#define POC_PAGE		(4096)
#define POC_IMG_WR_SIZE	POC_FA7_TOTAL_SIZE

enum {
	POC_OP_NONE = 0,
	POC_OP_ERASE = 1,
	POC_OP_WRITE = 2,
	POC_OP_READ = 3,
	POC_OP_CANCEL = 4,
	POC_OP_CHECKSUM = 5,
	POC_OP_CHECKPOC = 6,
	POC_OP_SECTOR_ERASE = 7,
	POC_OP_IMG_READ = 10,
	POC_OP_IMG_WRITE = 11,
	POC_OP_DIM_READ = 12,
	POC_OP_DIM_WRITE = 13,
	POC_OP_DIM_VALID = 14,
	POC_OP_DIM_CHKSUM = 15,
	POC_OP_BACKUP = 17,
	POC_OP_SELF_TEST = 18,
	POC_OP_READ_TEST = 19,
	POC_OP_WRITE_TEST = 20,
	POC_OP_IMG_READ_TEST = 21,
	POC_OP_DIM_READ_TEST = 22,
	POC_OP_DIM_READ_FROM_FILE,
	POC_OP_MTP_READ,
	POC_OP_MCD_READ,
	MAX_POC_OP,
};

#define IS_VALID_POC_OP(_op_)	\
	((_op_) > POC_OP_NONE && (_op_) < MAX_POC_OP)

#define IS_POC_OP_ACCESS_FLASH(_op_)	\
	((_op_) == POC_OP_ERASE ||\
	 (_op_) == POC_OP_WRITE ||\
	 (_op_) == POC_OP_READ ||\
	 (_op_) == POC_OP_BACKUP ||\
	 (_op_) == POC_OP_SELF_TEST)

enum poc_state {
	POC_STATE_NONE,
	POC_STATE_FLASH_EMPTY,
	POC_STATE_FLASH_FILLED,
	POC_STATE_ER_START,
	POC_STATE_ER_PROGRESS,
	POC_STATE_ER_COMPLETE,
	POC_STATE_ER_FAILED,
	POC_STATE_WR_START,
	POC_STATE_WR_PROGRESS,
	POC_STATE_WR_COMPLETE,
	POC_STATE_WR_FAILED,
	MAX_POC_STATE,
};

enum {
	/* poc erase */
	POC_ERASE_ENTER_SEQ,
	POC_ERASE_SEQ,
	POC_ERASE_4K_SEQ,
	POC_ERASE_32K_SEQ,
	POC_ERASE_64K_SEQ,
	POC_ERASE_EXIT_SEQ,

	/* poc write */
	POC_WRITE_ENTER_SEQ,
	POC_WRITE_STT_SEQ,
	POC_WRITE_DAT_SEQ,
	POC_WRITE_END_SEQ,
	POC_WRITE_EXIT_SEQ,

	/* poc read */
	POC_READ_PRE_ENTER_SEQ,
	POC_READ_ENTER_SEQ,
	POC_READ_DAT_SEQ,
	POC_READ_EXIT_SEQ,

	/* if necessary, add new seq */
	MAX_POC_SEQ,
};

struct panel_poc_info {
	bool enabled;
	bool erased;

	enum poc_state state;

	u8 poc;

	u32 waddr;
	u8 *wdata;
	u32 wdata_len;

	u8 *wbuf;
	u32 wpos;
	u32 wsize;

#ifdef CONFIG_DISPLAY_USE_INFO
	int total_failcount;
	int total_trycount;
	int erase_trycount;
	int erase_failcount;
	int write_trycount;
	int write_failcount;
	int read_trycount;
	int read_failcount;
#endif
};

struct panel_poc_device {
	struct miscdevice dev;
	__u64 count;
	unsigned int flags;
	atomic_t cancel;
	bool opened;
	bool read;
	unsigned int		enable;
	struct mutex			*lock;
	struct dsim_device		*dsim;

	struct panel_poc_info poc_info;
#ifdef CONFIG_DISPLAY_USE_INFO
	struct notifier_block poc_notif;
#endif
	struct notifier_block	fb_notif;

	unsigned int poc_op;
};

#define IOC_GET_POC_STATUS	_IOR('A', 100, __u32)		/* 0:NONE, 1:ERASED, 2:WROTE, 3:READ */
#define IOC_GET_POC_CHKSUM	_IOR('A', 101, __u32)		/* 0:CHKSUM ERROR, 1:CHKSUM SUCCESS */
#define IOC_GET_POC_CSDATA	_IOR('A', 102, __u32)		/* CHKSUM DATA 4 Bytes */
#define IOC_GET_POC_ERASED	_IOR('A', 103, __u32)		/* 0:NONE, 1:NOT ERASED, 2:ERASED */
#define IOC_GET_POC_FLASHED	_IOR('A', 104, __u32)		/* 0:NOT POC FLASHED(0x53), 1:POC FLAHSED(0x52) */
#define IOC_SET_POC_ERASE	_IOR('A', 110, __u32)		/* ERASE POC FLASH */
#define IOC_SET_POC_TEST	_IOR('A', 112, __u32)		/* POC FLASH TEST - ERASE/WRITE/READ/COMPARE */

static const u8 KEY1_ENABLE[] = { 0xF0, 0x5A, 0x5A };
static const u8 KEY1_DISABLE[] = { 0xF0, 0xA5, 0xA5 };
static const u8 KEY2_ENABLE[] = { 0xF1, 0xF1, 0xA2 };
static const u8 KEY2_DISABLE[] = { 0xF1, 0xA5, 0xA5 };
static const u8 KEY3_ENABLE[] = { 0xFC, 0x5A, 0x5A };
static const u8 KEY3_DISABLE[] = { 0xFC, 0xA5, 0xA5 };

static const u8 PGM_ENABLE[] = { 0xC0, 0x02 };
static const u8 PGM_DISABLE[] = { 0xC0, 0x00 };
static const u8 EXEC[] = { 0xC0, 0x03 };

static const u8 PRE_01[] = { 0xEB, 0x4E };
static const u8 PRE_02_GP[] = { 0xB0, 0x06 };
static const u8 PRE_02[] = { 0xEB, 0x6A };

static const u8 RW_CTRL_01[] = { 0xC1, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
static const u8 RW_CTRL_02[] = { 0xC1, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x10 };

static const u8 WR_ENABLE[] = { 0xC1, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const u8 WR_END[] = { 0xC1, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01 };

static const u8 ER_CTRL_01[] = { 0xC1, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };
static const u8 ER_CTRL_02[] = { 0xC1, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x10, 0x04 };

static const u8 ER_ENABLE[] = { 0xC1, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };

int panel_poc_probe(struct panel_poc_device *poc_dev);
int poc_erase(struct panel_poc_device *poc_dev, int addr, int len);
#endif
