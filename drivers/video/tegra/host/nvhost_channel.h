/*
 * drivers/video/tegra/host/nvhost_channel.h
 *
 * Tegra Graphics Host Channel
 *
 * Copyright (c) 2010-2014, NVIDIA Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NVHOST_CHANNEL_H
#define __NVHOST_CHANNEL_H

#include <linux/cdev.h>
#include <linux/io.h>
#include "nvhost_cdma.h"

#define NVHOST_MAX_WAIT_CHECKS		256
#define NVHOST_MAX_GATHERS		512
#define NVHOST_MAX_HANDLES		1280
#define NVHOST_MAX_POWERGATE_IDS	2
#define NVHOST_INVALID_CHANNEL		-1

struct nvhost_master;
struct platform_device;
struct nvhost_channel;
struct nvhost_hwctx;

struct nvhost_channel_ops {
	const char *soc_name;
	int (*init)(struct nvhost_channel *,
		    struct nvhost_master *);
	int (*submit)(struct nvhost_job *job);
	int (*save_context)(struct nvhost_channel *channel);
	int (*init_gather_filter)(struct nvhost_channel *ch);
};

struct nvhost_channel {
	struct nvhost_channel_ops ops;
	int refcount;
	int chid;
	int dev_chid;
	struct mutex submitlock;
	void __iomem *aperture;
	struct nvhost_hwctx *cur_ctx;
	struct platform_device *dev;
	struct nvhost_hwctx_handler *ctxhandler;
	struct nvhost_cdma cdma;

	/* the address space block here
	 * belongs to the module. but for
	 * now just keep it here */
	struct nvhost_as *as;

	/* error notificatiers used channel submit timeout */
	struct dma_buf *error_notifier_ref;
	struct nvhost_notification *error_notifier;
	void *error_notifier_va;
};

#define channel_op(ch)		(ch->ops)

int nvhost_alloc_channels(struct nvhost_master *host);
int nvhost_channel_map(struct nvhost_device_data *pdata,
			struct nvhost_channel **ch);
int nvhost_channel_unmap(struct nvhost_channel *ch);
int nvhost_channel_release(struct nvhost_device_data *pdata);
int nvhost_channel_list_free(struct nvhost_master *host);
struct nvhost_channel *nvhost_check_channel(struct nvhost_device_data *pdata);
int nvhost_channel_init(struct nvhost_channel *ch,
	struct nvhost_master *dev);
int nvhost_channel_submit(struct nvhost_job *job);
void nvhost_set_notifier(struct nvhost_channel *ch, __u32 error);
void nvhost_free_error_notifiers(struct nvhost_channel *ch);

void nvhost_getchannel(struct nvhost_channel *ch);
void nvhost_putchannel(struct nvhost_channel *ch, int cnt);
int nvhost_channel_suspend(struct nvhost_channel *ch);

int nvhost_channel_read_reg(struct nvhost_channel *channel,
	struct nvhost_hwctx *hwctx,
	u32 offset, u32 *value);

struct nvhost_channel *nvhost_alloc_channel_internal(int chindex,
	int max_channels);

int nvhost_channel_save_context(struct nvhost_channel *ch);
void nvhost_channel_init_gather_filter(struct nvhost_channel *ch);

struct nvhost_hwctx *nvhost_channel_get_file_hwctx(int fd);

struct nvhost_hwctx_handler *nvhost_alloc_hwctx_handler(u32 syncpt,
	u32 waitbase, struct nvhost_channel *ch);

#endif
