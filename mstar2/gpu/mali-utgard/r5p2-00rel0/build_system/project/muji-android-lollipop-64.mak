#
# Copyright (C) 2010-2016 MStar Semiconductor, Inc. All rights reserved.
# 
# This program is free software and is provided to you under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
# 
# A copy of the licence is included with the program, and can also be obtained from Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

include $(if $(KBUILD_EXTMOD),$(KBUILD_EXTMOD)/,)../build_system/platform/muji.mak

# arch
ARCH=arm64

# MStar build options/flags
MALI_IRQ := E_IRQEXPH_G3D2MCU
HAVE_ANDROID_OS = 1
MSTAR_VALLOC_PAGE_TABLE_DIR = 1
MSTAR_PRE_ALLOC_TABLE_PAGE = 1

# project build flags
CONFIG_BUILDFLAGS :=
