cmd_arch/arm/arm-boards/plat-mstar/headsmp.o := arm-none-linux-gnueabi-gcc -Wp,-MD,arch/arm/arm-boards/plat-mstar/.headsmp.o.d  -nostdinc -isystem /opt/arm-2012.09/bin/../lib/gcc/arm-none-linux-gnueabi/4.7.2/include -I/home/wenshuai.xi/SI_1/kernel/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/wenshuai.xi/SI_1/kernel/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/wenshuai.xi/SI_1/kernel/include/uapi -Iinclude/generated/uapi -include /home/wenshuai.xi/SI_1/kernel/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/include -Idrivers/mstar2 -Iarch/arm/arm-boards/generic/include/ -Iarch/arm/arm-boards/../../../../mstar2/hal/k6/cpu/arm//include -Iarch/arm/arm-boards/plat-mstar//include -Iarch/arm/arm-boards/../../../../mstar2/hal/k6/cpu/arm/ -Iarch/arm/arm-boards/plat-mstar/ -I drivers/mstar  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float         -c -o arch/arm/arm-boards/plat-mstar/headsmp.o arch/arm/arm-boards/plat-mstar/headsmp.S

source_arch/arm/arm-boards/plat-mstar/headsmp.o := arch/arm/arm-boards/plat-mstar/headsmp.S

deps_arch/arm/arm-boards/plat-mstar/headsmp.o := \
  /home/wenshuai.xi/SI_1/kernel/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  /home/wenshuai.xi/SI_1/kernel/arch/arm/include/asm/linkage.h \
  include/linux/init.h \
    $(wildcard include/config/broken/rodata.h) \
  include/linux/types.h \
    $(wildcard include/config/mp/debug/tool/changelist.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  include/uapi/linux/types.h \
  arch/arm/include/generated/asm/types.h \
  /home/wenshuai.xi/SI_1/kernel/include/uapi/asm-generic/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \

arch/arm/arm-boards/plat-mstar/headsmp.o: $(deps_arch/arm/arm-boards/plat-mstar/headsmp.o)

$(deps_arch/arm/arm-boards/plat-mstar/headsmp.o):
