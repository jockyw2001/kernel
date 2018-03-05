/* Copyright (C) 2014 Cadence Design Systems.  All rights reserved           */
/* THIS FILE IS AUTOMATICALLY GENERATED BY CADENCE BLUEPRINT, DO NOT EDIT    */
/*                                                                           */


#ifndef __REG_PCIE_UDMA_ADDRMAP_H__
#define __REG_PCIE_UDMA_ADDRMAP_H__

#include "pcie_udma_addrmap_macro.h"

struct pcie_udma_addrmap__udma_one_ch_regs {
  volatile uint32_t channel_ctrl;                 /*        0x0 - 0x4        */
  volatile uint32_t channel_sp_l;                 /*        0x4 - 0x8        */
  volatile uint32_t channel_sp_u;                 /*        0x8 - 0xc        */
  volatile uint32_t channel_attr_l;               /*        0xc - 0x10       */
  volatile uint32_t channel_attr_u;               /*       0x10 - 0x14       */
};

struct pcie_udma_addrmap__udma_common_regs {
  volatile uint32_t common_udma_int;              /*        0x0 - 0x4        */
  volatile uint32_t common_udma_int_ena;          /*        0x4 - 0x8        */
  volatile uint32_t common_udma_int_dis;          /*        0x8 - 0xc        */
  volatile uint32_t common_udma_ib_ecc_uncorrectable_errors;
                                                  /*        0xc - 0x10       */
  volatile uint32_t common_udma_ib_ecc_correctable_errors;
                                                  /*       0x10 - 0x14       */
  volatile uint32_t common_udma_ob_ecc_uncorrectable_errors;
                                                  /*       0x14 - 0x18       */
  volatile uint32_t common_udma_ob_ecc_correctable_errors;
                                                  /*       0x18 - 0x1c       */
  volatile char pad__0[0x3c];                     /*       0x1c - 0x58       */
  volatile uint32_t common_udma_cap_ver;          /*       0x58 - 0x5c       */
  volatile uint32_t common_udma_config;           /*       0x5c - 0x60       */
};

struct pcie_udma_addrmap {
  struct pcie_udma_addrmap__udma_one_ch_regs dma_ch0;
                                                  /*        0x0 - 0x14       */
  struct pcie_udma_addrmap__udma_one_ch_regs dma_ch1;
                                                  /*       0x14 - 0x28       */
  volatile char pad__0[0x78];                     /*       0x28 - 0xa0       */
  struct pcie_udma_addrmap__udma_common_regs dma_common;
                                                  /*       0xa0 - 0x100      */
};

#endif /* __REG_PCIE_UDMA_ADDRMAP_H__ */