///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2007 MStar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//#include <linux/config.h>
#include <generated/autoconf.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include "chip_int.h"

#define ENABLE_THIRD_EHC
#define ENABLE_FOURTH_EHC

#if  defined(CONFIG_USB_XHCI_HCD) || defined(CONFIG_USB_XHCI_HCD_MODULE) 
#define ENABLE_XHC
#endif

#define UTMI_BASE_ADDRESS_START		0xFD207500
#define UTMI_BASE_ADDRESS_END		0xFD2075FC
#define USB_HOST20_ADDRESS_START	0xFD204800
#define USB_HOST20_ADDRESS_END		0xFD2049FC
//tony add for 2st EHCI
#define SECOND_UTMI_BASE_ADDRESS_START	0xFD207400
#define SECOND_UTMI_BASE_ADDRESS_END	0xFD2074FC
#define SECOND_USB_HOST20_ADDRESS_START	0xFD201A00
#define SECOND_USB_HOST20_ADDRESS_END	0xFD201BFC

#define THIRD_UTMI_BASE_ADDRESS_START	0xFD207200
#define THIRD_UTMI_BASE_ADDRESS_END	0xFD2072FC
#define THIRD_USB_HOST20_ADDRESS_START	0xFD227200
#define THIRD_USB_HOST20_ADDRESS_END	0xFD2273FC

#define FOURTH_UTMI_BASE_ADDRESS_START	0xFD244100
#define FOURTH_UTMI_BASE_ADDRESS_END	0xFD2441FC
#define FOURTH_USB_HOST20_ADDRESS_START	0xFD244C00
#define FOURTH_USB_HOST20_ADDRESS_END	0xFD244DFC

#define XHCI_ADDRESS_START           0xFD320000
#define XHCI_ADDRESS_END             0xFD327fff
#define U3PHY_ADDRESS_START          0xFD244200
#define U3PHY_ADDRESS_END            0xFD244BFF


static struct resource Mstar_usb_ehci_resources[] = {
	[0] = {
		.start		= UTMI_BASE_ADDRESS_START,
		.end		= UTMI_BASE_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= USB_HOST20_ADDRESS_START,
		.end		= USB_HOST20_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= E_IRQ_UHC,
		.end		= E_IRQ_UHC,
		.flags		= IORESOURCE_IRQ,
	},
};

//tony add for 2st EHCI
static struct resource Second_Mstar_usb_ehci_resources[] = {
	[0] = {
		.start		= SECOND_UTMI_BASE_ADDRESS_START,
		.end		= SECOND_UTMI_BASE_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
        },
	[1] = {
		.start		= SECOND_USB_HOST20_ADDRESS_START,
		.end		= SECOND_USB_HOST20_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= E_IRQEXPL_UHC1,
		.end		= E_IRQEXPL_UHC1,
		.flags		= IORESOURCE_IRQ,
	},
};

#ifdef ENABLE_THIRD_EHC
static struct resource Third_Mstar_usb_ehci_resources[] = {
	[0] = {
		.start		= THIRD_UTMI_BASE_ADDRESS_START,
		.end		= THIRD_UTMI_BASE_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= THIRD_USB_HOST20_ADDRESS_START,
		.end		= THIRD_USB_HOST20_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= E_IRQEXPL_UHC2,
		.end		= E_IRQEXPL_UHC2,
		.flags		= IORESOURCE_IRQ,
	},
};
#endif

#ifdef ENABLE_FOURTH_EHC
static struct resource Fourth_Mstar_usb_ehci_resources[] = {
	[0] = {
		.start		= FOURTH_UTMI_BASE_ADDRESS_START,
		.end		= FOURTH_UTMI_BASE_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= FOURTH_USB_HOST20_ADDRESS_START,
		.end		= FOURTH_USB_HOST20_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= E_IRQ_U3_DPHY,
		.end		= E_IRQ_U3_DPHY,
		.flags		= IORESOURCE_IRQ,
	},
};
#endif

/* The dmamask must be set for EHCI to work */
static u64 ehci_dmamask = ~(u32)0;

static struct platform_device Mstar_usb_ehci_device = {
	.name           = "Mstar-ehci-1",
	.id             = 0,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff, //tony add for limit DMA range
	},
	.num_resources	= ARRAY_SIZE(Mstar_usb_ehci_resources),
	.resource	= Mstar_usb_ehci_resources,
};
//tony add for 2st EHCI
static struct platform_device Second_Mstar_usb_ehci_device = {
	.name		= "Mstar-ehci-2",
	.id		= 1,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,    //tony add for limit DMA range
	},
	.num_resources	= ARRAY_SIZE(Second_Mstar_usb_ehci_resources),
	.resource	= Second_Mstar_usb_ehci_resources,
};
#ifdef ENABLE_THIRD_EHC
static struct platform_device Third_Mstar_usb_ehci_device = {
	.name		= "Mstar-ehci-3",
	.id		= 2,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,    //tony add for limit DMA range
	},
	.num_resources	= ARRAY_SIZE(Third_Mstar_usb_ehci_resources),
	.resource	= Third_Mstar_usb_ehci_resources,
};
#endif
#ifdef ENABLE_FOURTH_EHC
static struct platform_device Fourth_Mstar_usb_ehci_device = {
	.name		= "Mstar-ehci-4",
	.id		= 3,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,    //tony add for limit DMA range
	},
	.num_resources	= ARRAY_SIZE(Fourth_Mstar_usb_ehci_resources),
	.resource	= Fourth_Mstar_usb_ehci_resources,
};
#endif

#ifdef ENABLE_XHC
//-----------------------------------------
//   xHCI platform device
//-----------------------------------------
static struct resource Mstar_usb_xhci_resources[] = {
	[0] = {
		.start		= U3PHY_ADDRESS_START,
		.end		= U3PHY_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= XHCI_ADDRESS_START,
		.end		= XHCI_ADDRESS_END,
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= E_IRQ_UHC30,
		.end		= E_IRQ_UHC30,
		.flags		= IORESOURCE_IRQ,
	},
};

static u64 xhci_dmamask = 0xffffffff;

static struct platform_device Mstar_usb_xhci_device = {
	.name		= "Mstar-xhci-1",
	.id		= 0,
	.dev = {
		.dma_mask		= &xhci_dmamask,
		.coherent_dma_mask	= 0xffffffff, 
	},
	.num_resources	= ARRAY_SIZE(Mstar_usb_xhci_resources),
	.resource	= Mstar_usb_xhci_resources,

};
//---------------------------------------------------------------
#endif

static struct platform_device *Mstar_platform_devices[] = {
	&Mstar_usb_ehci_device,
	&Second_Mstar_usb_ehci_device,   //tony add for 2st EHCI
#ifdef ENABLE_THIRD_EHC
	&Third_Mstar_usb_ehci_device,
#endif
#ifdef ENABLE_FOURTH_EHC
	&Fourth_Mstar_usb_ehci_device,
#endif
#ifdef ENABLE_XHC
    &Mstar_usb_xhci_device,    
#endif
};

int Mstar_ehc_platform_init(void)
{
	return platform_add_devices(Mstar_platform_devices, ARRAY_SIZE(Mstar_platform_devices));
}