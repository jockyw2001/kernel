/*
 * drivers/video/tegra/host/t124/t124.c
 *
 * Tegra Graphics Init for T124 Architecture Chips
 *
 * Copyright (c) 2011-2014, NVIDIA CORPORATION. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.>
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/slab.h>
#include <linux/tegra-powergate.h>
#include <linux/tegra-fuse.h>

#include <mach/mc.h>

#include "dev.h"
#include "nvhost_channel.h"
#include "nvhost_job.h"
#include "class_ids.h"
#include "t124.h"
#include "host1x/host1x.h"
#include "hardware_t124.h"
#include "syncpt_t124.h"
#include "tsec/tsec.h"
#include "flcn/flcn.h"
#include "vi/vi.h"
#include "isp/isp.h"
#include "scale3d.h"
#include "chip_support.h"
#include "nvhost_scale.h"

#include "../../../../arch/arm/mach-tegra/iomap.h"

#define HOST_EMC_FLOOR 300000000
#define VI_CLOCKGATE_DELAY 60
#define VI_POWERGATE_DELAY 500
#define ISP_CLOCKGATE_DELAY 60
#define ISP_POWERGATE_DELAY 500
#define TSEC_POWERGATE_DELAY 500
#define HOST1X_POWERGATE_DELAY 50

/* Host1x driver matches module IDs while setting a
 * particular clock, This ID is used for EMC module.
 */
#define TEGRA_HOST1X_EMC_MODULE_ID 75

static struct resource tegra_host1x04_resources[] = {
	{
		.start = TEGRA_HOST1X_BASE,
		.end = TEGRA_HOST1X_BASE + TEGRA_HOST1X_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = INT_HOST1X_MPCORE_SYNCPT,
		.end = INT_HOST1X_MPCORE_SYNCPT,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = INT_HOST1X_MPCORE_GENERAL,
		.end = INT_HOST1X_MPCORE_GENERAL,
		.flags = IORESOURCE_IRQ,
	},
};

static struct host1x_device_info host1x04_info = {
	.nb_channels	= T124_NVHOST_NUMCHANNELS,
	.nb_pts		= NV_HOST1X_SYNCPT_NB_PTS,
	.nb_mlocks	= NV_HOST1X_NB_MLOCKS,
	.nb_bases	= NV_HOST1X_SYNCPT_NB_BASES,
};

struct nvhost_device_data t124_host1x_info = {
	.clocks		= {{"host1x", 81600000}, {"actmon", UINT_MAX} },
	NVHOST_MODULE_NO_POWERGATE_IDS,
	.can_powergate   = true,
	.powergate_delay = HOST1X_POWERGATE_DELAY,
	.private_data	= &host1x04_info,
	.finalize_poweron = nvhost_host1x_finalize_poweron,
	.prepare_poweroff = nvhost_host1x_prepare_poweroff,
};


static struct platform_device tegra_host1x04_device = {
	.name		= "host1x",
	.id		= -1,
	.resource	= tegra_host1x04_resources,
	.num_resources	= ARRAY_SIZE(tegra_host1x04_resources),
	.dev            = {
		.platform_data = &t124_host1x_info,
	},
};


static struct resource isp_resources[] = {
	{
		.name = "regs",
		.start = TEGRA_ISP_BASE,
		.end = TEGRA_ISP_BASE + TEGRA_ISP_SIZE - 1,
		.flags = IORESOURCE_MEM,
	}
};

static struct platform_device tegra_isp01b_device;
struct nvhost_device_data t124_isp_info = {
	.num_channels	= 1,
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid        = NVHOST_MODULE_ISP,
	.modulemutexes   = {NVMODMUTEX_ISP_0},
	.exclusive       = true,
	.keepalive       = true,
	.powergate_ids   = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate   = true,
	.clockgate_delay = ISP_CLOCKGATE_DELAY,
	.powergate_delay = ISP_POWERGATE_DELAY,
	.clocks          = {
		{"isp", UINT_MAX, 0, TEGRA_MC_CLIENT_ISP},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID},
		{"sclk", 80000000} },
	.finalize_poweron = nvhost_isp_t124_finalize_poweron,
	.ctrl_ops         = &tegra_isp_ctrl_ops,
	.alloc_hwctx_handler = nvhost_alloc_hwctx_handler,
};
static struct platform_device tegra_isp01_device = {
	.name          = "isp",
	.resource      = isp_resources,
	.num_resources = ARRAY_SIZE(isp_resources),
	.dev           = {
		.platform_data = &t124_isp_info,
	},
};

static struct resource ispb_resources[] = {
	{
		.name = "regs",
		.start = TEGRA_ISPB_BASE,
		.end = TEGRA_ISPB_BASE + TEGRA_ISPB_SIZE - 1,
		.flags = IORESOURCE_MEM,
	}
};


struct nvhost_device_data t124_ispb_info = {
	.num_channels	= 1,
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid        = (1 << 16) | NVHOST_MODULE_ISP,
	.modulemutexes   = {NVMODMUTEX_ISP_1},
	.exclusive       = true,
	.keepalive       = true,
	.powergate_ids   = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate   = true,
	.clockgate_delay = ISP_CLOCKGATE_DELAY,
	.powergate_delay = ISP_POWERGATE_DELAY,
	.clocks          = {
		{"isp", UINT_MAX, 0, TEGRA_MC_CLIENT_ISPB},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID},
		{"sclk", 80000000} },
	.finalize_poweron = nvhost_isp_t124_finalize_poweron,
	.ctrl_ops         = &tegra_isp_ctrl_ops,
	.alloc_hwctx_handler = nvhost_alloc_hwctx_handler,
};

static struct platform_device tegra_isp01b_device = {
	.name          = "isp",
	.id            = 1, /* .1 on the dev node */
	.resource      = ispb_resources,
	.num_resources = ARRAY_SIZE(ispb_resources),
	.dev  = {
		.platform_data = &t124_ispb_info,
	},
};

static struct resource vi_resources[] = {
	{
		.name = "regs",
		.start = TEGRA_VI_BASE,
		.end = TEGRA_VI_BASE + TEGRA_VI_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device tegra_vi01b_device;
struct nvhost_device_data t124_vi_info = {
	.num_channels	= 1,
	/* FIXME: resolve powergating dependency with DIS */
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid         = NVHOST_MODULE_VI,
	.modulemutexes    = {NVMODMUTEX_VI_0},
	.exclusive        = true,
	.keepalive       = true,
	.powergate_ids    = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate    = true,
	.clockgate_delay  = VI_CLOCKGATE_DELAY,
	.powergate_delay  = VI_POWERGATE_DELAY,
	.clocks           = {
		{"vi", UINT_MAX, 0},
		{"csi", 0},
		{"cilab", 102000000},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID},
		{"sclk", 80000000} },
	.prepare_poweroff = nvhost_vi_prepare_poweroff,
	.finalize_poweron = nvhost_vi_finalize_poweron,
	.ctrl_ops         = &tegra_vi_ctrl_ops,
	.reset            = nvhost_vi_reset,
	.slave         = &tegra_vi01b_device,
	.alloc_hwctx_handler = nvhost_alloc_hwctx_handler,
};
EXPORT_SYMBOL(t124_vi_info);

static struct platform_device tegra_vi01_device = {
	.name		= "vi",
	.resource	= vi_resources,
	.num_resources	= ARRAY_SIZE(vi_resources),
	.dev		= {
		.platform_data = &t124_vi_info,
	},
};

struct nvhost_device_data t124_vib_info = {
	.num_channels	= 1,
	/* FIXME: resolve powergating dependency with DIS */
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid         = (1 << 16 | NVHOST_MODULE_VI),
	.modulemutexes    = {NVMODMUTEX_VI_1},
	.exclusive        = true,
	.keepalive       = true,
	.powergate_ids    = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate    = true,
	.clockgate_delay  = VI_CLOCKGATE_DELAY,
	.powergate_delay  = VI_POWERGATE_DELAY,
	.clocks           = {
		{"vi", UINT_MAX},
		{"csi", 0},
		{"cilcd", 102000000},
		{"cile", 102000000},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID},
		{"sclk", 80000000} },
	.prepare_poweroff = nvhost_vi_prepare_poweroff,
	.finalize_poweron = nvhost_vi_finalize_poweron,
	.ctrl_ops         = &tegra_vi_ctrl_ops,
	.master           = &tegra_vi01_device,
	.reset            = nvhost_vi_reset,
	.alloc_hwctx_handler = nvhost_alloc_hwctx_handler,
};

static struct platform_device tegra_vi01b_device = {
	.name		= "vi",
	.id		= 1, /* .1 on the dev node */
	.dev		= {
		.platform_data = &t124_vib_info,
	},
};

static struct resource msenc_resources[] = {
	{
		.name = "regs",
		.start = TEGRA_MSENC_BASE,
		.end = TEGRA_MSENC_BASE + TEGRA_MSENC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

struct nvhost_device_data t124_msenc_info = {
	.num_channels	= 1,
	.version	= NVHOST_ENCODE_FLCN_VER(3, 1),
	.class		= NV_VIDEO_ENCODE_MSENC_CLASS_ID,
	.clocks		= {{"msenc", UINT_MAX, 0, TEGRA_MC_CLIENT_MSENC},
			  {"emc", HOST_EMC_FLOOR, TEGRA_HOST1X_EMC_MODULE_ID} },
	NVHOST_DEFAULT_CLOCKGATE_DELAY,
	.moduleid	= NVHOST_MODULE_MSENC,
	.powergate_ids	= { TEGRA_POWERGATE_MPE, -1 },
	.powergate_delay = 100,
	.can_powergate	= true,
	.init           = nvhost_flcn_init,
	.deinit         = nvhost_flcn_deinit,
	.finalize_poweron = nvhost_flcn_boot,
	.scaling_init	= nvhost_scale_init,
	.scaling_deinit	= nvhost_scale_deinit,
	.actmon_regs	= HOST1X_CHANNEL_ACTMON1_REG_BASE,
	.actmon_enabled	= true,
	.firmware_name	= "nvhost_msenc031.fw",
};

struct platform_device tegra_msenc03_device = {
	.name	       = "msenc",
	.id	       = -1,
	.resource      = msenc_resources,
	.num_resources = ARRAY_SIZE(msenc_resources),
	.dev           = {
		.platform_data = &t124_msenc_info,
	},
};

static struct resource tsec_resources[] = {
	{
		.name = "regs",
		.start = TEGRA_TSEC_BASE,
		.end = TEGRA_TSEC_BASE + TEGRA_TSEC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

struct nvhost_device_data t124_tsec_info = {
	.num_channels	= 1,
	.version       = NVHOST_ENCODE_TSEC_VER(1, 0),
	.class         = NV_TSEC_CLASS_ID,
	.exclusive     = true,
	.clocks	       = {{"tsec", UINT_MAX, 0, TEGRA_MC_CLIENT_TSEC},
			 {"emc", HOST_EMC_FLOOR} },
	NVHOST_MODULE_NO_POWERGATE_IDS,
	NVHOST_DEFAULT_CLOCKGATE_DELAY,
	.can_powergate    = true,
	.powergate_delay = TSEC_POWERGATE_DELAY,
	.keepalive       = true,
	.moduleid      = NVHOST_MODULE_TSEC,
	.init          = nvhost_tsec_init,
	.deinit        = nvhost_tsec_deinit,
	.finalize_poweron = nvhost_tsec_finalize_poweron,
	.prepare_poweroff = nvhost_tsec_prepare_poweroff,
};

static struct platform_device tegra_tsec01_device = {
	.name		= "tsec",
	.id		= -1,
	.resource	= tsec_resources,
	.num_resources	= ARRAY_SIZE(tsec_resources),
	.dev		= {
		.platform_data = &t124_tsec_info,
	},
};

#ifdef CONFIG_ARCH_TEGRA_VIC
static struct resource vic03_resources[] = {
	{
	.name = "base",
	.start = TEGRA_VIC_BASE,
	.end = TEGRA_VIC_BASE + TEGRA_VIC_SIZE - 1,
	.flags = IORESOURCE_MEM,
	},
};

struct nvhost_device_data t124_vic_info = {
	.num_channels	= 1,
	.modulemutexes		= {NVMODMUTEX_VIC},
	.clocks			= {{"vic03", UINT_MAX, 0, TEGRA_MC_CLIENT_VIC},
				  {"emc", UINT_MAX} },
	.version = NVHOST_ENCODE_FLCN_VER(3, 0),
	NVHOST_MODULE_NO_POWERGATE_IDS,
	NVHOST_DEFAULT_CLOCKGATE_DELAY,
	.moduleid      = NVHOST_MODULE_VIC,
	.alloc_hwctx_handler = nvhost_vic03_alloc_hwctx_handler,
	.can_powergate		= true,
	.powergate_delay	= 500,
	.powergate_ids		= { TEGRA_POWERGATE_VIC, -1 },
	.init			= nvhost_flcn_init,
	.deinit			= nvhost_flcn_deinit,
	.alloc_hwctx_handler	= nvhost_vic03_alloc_hwctx_handler,
	.finalize_poweron	= nvhost_vic_finalize_poweron,
	.prepare_poweroff	= nvhost_vic_prepare_poweroff,
	.scaling_init		= nvhost_scale3d_init,
	.scaling_deinit		= nvhost_scale3d_deinit,
	.busy			= nvhost_scale_notify_busy,
	.idle			= nvhost_scale_notify_idle,
	.suspend_ndev		= nvhost_scale3d_suspend,
	.scaling_post_cb	= &nvhost_scale3d_callback,
	.devfreq_governor	= "nvhost_podgov",
	.actmon_regs		= HOST1X_CHANNEL_ACTMON2_REG_BASE,
	.actmon_enabled		= true,
	.linear_emc		= true,
	.firmware_name		= "vic03_ucode.bin",
};

struct platform_device tegra_vic03_device = {
	.name	       = "vic03",
	.num_resources = 1,
	.resource      = vic03_resources,
	.dev           = {
		.platform_data = &t124_vic_info,
	},
};
#endif

/*
 * T132 overrides for platform data.
 */

struct nvhost_device_data t132_isp_info = {
	.num_channels	= 1,
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid        = NVHOST_MODULE_ISP,
	.modulemutexes   = {NVMODMUTEX_ISP_0},
	.exclusive       = true,
	.keepalive       = true,
	.powergate_ids   = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate   = true,
	.clockgate_delay = ISP_CLOCKGATE_DELAY,
	.powergate_delay = ISP_POWERGATE_DELAY,
	.clocks          = {
		{"isp", UINT_MAX, 0, TEGRA_MC_CLIENT_ISP},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID} },
	.finalize_poweron = nvhost_isp_t124_finalize_poweron,
	.ctrl_ops         = &tegra_isp_ctrl_ops,
};

struct nvhost_device_data t132_ispb_info = {
	.num_channels	= 1,
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid        = (1 << 16) | NVHOST_MODULE_ISP,
	.modulemutexes   = {NVMODMUTEX_ISP_1},
	.exclusive       = true,
	.keepalive       = true,
	.powergate_ids   = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate   = true,
	.clockgate_delay = ISP_CLOCKGATE_DELAY,
	.powergate_delay = ISP_POWERGATE_DELAY,
	.clocks          = {
		{"isp", UINT_MAX, 0, TEGRA_MC_CLIENT_ISPB},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID} },
	.finalize_poweron = nvhost_isp_t124_finalize_poweron,
	.ctrl_ops         = &tegra_isp_ctrl_ops,
};

struct nvhost_device_data t132_vi_info = {
	.num_channels	= 1,
	/* FIXME: resolve powergating dependency with DIS */
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid         = NVHOST_MODULE_VI,
	.modulemutexes    = {NVMODMUTEX_VI_0},
	.exclusive        = true,
	.keepalive       = true,
	.powergate_ids    = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate    = true,
	.clockgate_delay  = VI_CLOCKGATE_DELAY,
	.powergate_delay  = VI_POWERGATE_DELAY,
	.clocks           = {
		{"vi", UINT_MAX, 0},
		{"csi", 0},
		{"cilab", 102000000},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID} },
	.prepare_poweroff = nvhost_vi_prepare_poweroff,
	.finalize_poweron = nvhost_vi_finalize_poweron,
	.ctrl_ops         = &tegra_vi_ctrl_ops,
	.reset            = nvhost_vi_reset,
	.slave         = &tegra_vi01b_device,
};

struct nvhost_device_data t132_vib_info = {
	.num_channels	= 1,
	/* FIXME: resolve powergating dependency with DIS */
	/* FIXME: control clocks from user space instead of hard-coding here */
	.moduleid         = (1 << 16 | NVHOST_MODULE_VI),
	.modulemutexes    = {NVMODMUTEX_VI_1},
	.exclusive        = true,
	.keepalive       = true,
	.powergate_ids    = {TEGRA_POWERGATE_VENC, -1},
	.can_powergate    = true,
	.clockgate_delay  = VI_CLOCKGATE_DELAY,
	.powergate_delay  = VI_POWERGATE_DELAY,
	.clocks           = {
		{"vi", UINT_MAX},
		{"csi", 0},
		{"cilcd", 102000000},
		{"cile", 102000000},
		{"emc", 0, TEGRA_HOST1X_EMC_MODULE_ID} },
	.prepare_poweroff = nvhost_vi_prepare_poweroff,
	.finalize_poweron = nvhost_vi_finalize_poweron,
	.ctrl_ops         = &tegra_vi_ctrl_ops,
	.master           = &tegra_vi01_device,
	.reset            = nvhost_vi_reset,
};

struct nvhost_device_data t132_msenc_info = {
	.num_channels	= 1,
	.version	= NVHOST_ENCODE_FLCN_VER(3, 1),
	.class		= NV_VIDEO_ENCODE_MSENC_CLASS_ID,
	.clocks		= {{"msenc", UINT_MAX, 0, TEGRA_MC_CLIENT_MSENC},
			  {"emc", HOST_EMC_FLOOR, TEGRA_HOST1X_EMC_MODULE_ID} },
	NVHOST_DEFAULT_CLOCKGATE_DELAY,
	.moduleid	= NVHOST_MODULE_MSENC,
	.powergate_ids	= { TEGRA_POWERGATE_MPE, -1 },
	.powergate_delay = 100,
	.can_powergate	= true,
	.init           = nvhost_flcn_init,
	.deinit         = nvhost_flcn_deinit,
	.finalize_poweron = nvhost_flcn_boot,
	.firmware_name	= "nvhost_msenc031.fw",
};

struct nvhost_device_data t132_tsec_info = {
	.num_channels	= 1,
	.version       = NVHOST_ENCODE_TSEC_VER(1, 0),
	.class         = NV_TSEC_CLASS_ID,
	.exclusive     = true,
	.clocks	       = {{"tsec", UINT_MAX, 0, TEGRA_MC_CLIENT_TSEC},
			 {"emc", HOST_EMC_FLOOR} },
	NVHOST_MODULE_NO_POWERGATE_IDS,
	NVHOST_DEFAULT_CLOCKGATE_DELAY,
	.moduleid      = NVHOST_MODULE_TSEC,
	.init          = nvhost_tsec_init,
	.deinit        = nvhost_tsec_deinit,
	.finalize_poweron = nvhost_tsec_finalize_poweron,
};

static struct {
	struct nvhost_device_data *from;
	struct nvhost_device_data *to;
} t132_override[] = {
	{&t124_isp_info, &t132_isp_info},
	{&t124_ispb_info, &t132_ispb_info},
	{&t124_vi_info, &t132_vi_info},
	{&t124_msenc_info, &t132_msenc_info},
	{&t124_tsec_info, &t132_tsec_info},
};

static struct platform_device *t124_devices[] = {
	&tegra_isp01_device,
	&tegra_isp01b_device,
	&tegra_vi01_device,
	&tegra_msenc03_device,
	&tegra_tsec01_device,
#if defined(CONFIG_ARCH_TEGRA_VIC)
	&tegra_vic03_device,
#endif
};


struct platform_device *tegra12_register_host1x_devices(void)
{
	int i = 0;
	struct platform_device *pdev;

	nvhost_dbg_fn("");

	/* register host1x device first */
	platform_device_register(&tegra_host1x04_device);
	tegra_host1x04_device.dev.parent = NULL;

	/* register clients with host1x device as parent */
	for (i = 0; i < ARRAY_SIZE(t124_devices); i++) {
		pdev = t124_devices[i];
		pdev->dev.parent = &tegra_host1x04_device.dev;
		platform_device_register(pdev);
	}

	return &tegra_host1x04_device;
}

#include "host1x/host1x_channel.c"

static void t124_set_nvhost_chanops(struct nvhost_channel *ch)
{
	if (ch)
		ch->ops = host1x_channel_ops;
}

int nvhost_init_t124_channel_support(struct nvhost_master *host,
       struct nvhost_chip_support *op)
{
	op->nvhost_dev.set_nvhost_chanops = t124_set_nvhost_chanops;

	return 0;
}

static void t124_remove_support(struct nvhost_chip_support *op)
{
	kfree(op->priv);
	op->priv = 0;
}

#include "host1x/host1x_cdma.c"
#include "host1x/host1x_debug.c"
#include "host1x/host1x_syncpt.c"
#include "host1x/host1x_intr.c"
#include "host1x/host1x_actmon_t124.c"

int nvhost_init_t124_support(struct nvhost_master *host,
       struct nvhost_chip_support *op)
{
	int i = 0;
	int err;
	struct t124 *t124 = 0;

	/* don't worry about cleaning up on failure... "remove" does it. */
	err = nvhost_init_t124_channel_support(host, op);
	if (err)
		return err;

	op->cdma = host1x_cdma_ops;
	op->push_buffer = host1x_pushbuffer_ops;
	op->debug = host1x_debug_ops;
	host->sync_aperture = host->aperture + HOST1X_CHANNEL_SYNC_REG_BASE;
	op->syncpt = host1x_syncpt_ops;
	op->intr = host1x_intr_ops;
	op->actmon = host1x_actmon_ops;

	t124 = kzalloc(sizeof(struct t124), GFP_KERNEL);
	if (!t124) {
		err = -ENOMEM;
		goto err;
	}

	t124->host = host;
	op->priv = t124;
	op->remove_support = t124_remove_support;

	if (tegra_get_chipid() == TEGRA_CHIPID_TEGRA13) {
		for (i = 0; i < ARRAY_SIZE(t132_override); i++) {
			struct nvhost_device_data *from = t132_override[i].from;
			struct nvhost_device_data *to = t132_override[i].to;

			/* replace the platform data by t132 data */
			*from = *to;
		}
	}

	return 0;

err:
	kfree(t124);

	op->priv = 0;
	op->remove_support = 0;
	return err;
}
