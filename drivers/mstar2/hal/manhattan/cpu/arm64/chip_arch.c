/*------------------------------------------------------------------------------
	Copyright (c) 2008 MStar Semiconductor, Inc.  All rights reserved.
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
    PROJECT: chip

	FILE NAME: arch/arm/mach-chip/chip_arch.c

    DESCRIPTION:
          Power Management Driver

    HISTORY:
         <Date>     <Author>    <Modification Description>
        2008/07/18  Fred Cheng  Add IO tables for ITCM and DTCM
        2008/07/22  Evan Chang  Add SD card init

------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    Include Files
------------------------------------------------------------------------------*/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <asm/memory.h>
#include <mach/io.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <asm/irq.h>
#include <chip_int.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <include/prom.h>
#include <mstar/mstar_chip.h>
#include <asm/cacheflush.h>
#ifdef CONFIG_WRITE_TO_ANDROID_MISC
#include <linux/reboot.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#endif

#ifdef CONFIG_WRITE_TO_ANDROID_MISC_NAND
#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stringify.h>
#include <linux/namei.h>
#include <linux/stat.h>
#include <linux/miscdevice.h>
#include <linux/log2.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#endif

#ifdef CONFIG_WRITE_TO_ANDROID_MISC_NAND
int Write_Mtd0_init(void);
int Mtd_erase_ctrl_blocks(void);
int Write_Mtd0_recovery(__u8* DataBuf,unsigned long u32Size);
#endif


#define XTAL 0x0000 /* 12M */
#define MPL  0x0400 /* 216M */
#define PLL  0x8000 /* freq. depends on setting */

void __init serial_init(void)
{
#ifdef CONFIG_SERIAL_8250
    struct uart_port s, u, k;

    memset(&s, 0, sizeof(s));
    s.type = PORT_16550;
    s.iobase = mstar_pm_base + (0x100980UL << 1); 
    s.irq = E_IRQ_UART0;
#ifdef CONFIG_MSTAR_ARM_BD_FPGA
    s.uartclk = 12000000; //FPGA
#elif defined(CONFIG_MSTAR_ARM_BD_GENERIC)
    s.uartclk = 123000000; //real chip
#endif
    s.iotype = 0;
    s.regshift = 0;
    s.fifosize = 16 ; // use the 8 byte depth FIFO well
    s.line = 0;
    

    if (early_serial_setup(&s) != 0) {
        printk(KERN_ERR "Serial(0) setup failed!\n");
    }

    memset(&u, 0, sizeof(u));
    u.type = PORT_16550;
    u.iobase = mstar_pm_base + (0x110600UL << 1); 
    u.irq = E_IRQEXPL_UART1;
    u.uartclk = 108000000;
    u.iotype = 0;
    u.regshift = 0;
    u.fifosize = 16 ; // use the 8 byte depth FIFO well
    u.line = 1;
    if (early_serial_setup(&u) != 0) {
    	printk(KERN_ERR "Serial piu_uart1 setup failed!\n");
    }

    memset(&k, 0, sizeof(k));
    k.type = PORT_16550;
    k.iobase = mstar_pm_base + (0x110680UL << 1);
    k.irq = E_IRQEXPH_UART2MCU;
    k.uartclk = 123000000;
    k.iotype = 0;
    k.regshift = 0;
    k.fifosize = 16 ; // use the 8 byte depth FIFO well
    k.line = 2;
    if (early_serial_setup(&k) != 0) {
    	printk(KERN_ERR "Serial fuart setup failed!\n");
    }
#endif
}

#ifdef CONFIG_WRITE_TO_ANDROID_MISC

#define REBOOT_RECOVERY_STR  "boot-recovery"
#define REBOOT_UPGRADEBOOTLOADER_STR  "update-radio/hboot"

static int recovery_sys_notify_reboot(struct notifier_block *this,
            unsigned long code, void *x)
{
    if(x && strlen((char*)x))
    {
        mm_segment_t old_fs;
        loff_t off;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        //printk(KERN_ERR "reboot cmd %s\n", (char*)x);
#if defined(CONFIG_WRITE_TO_ANDROID_MISC_EMMC) /* EMMC */
                {
                        struct file *fd = filp_open("/dev/block/platform/mstar_mci.0/by-name/misc", O_RDWR, 0);
                        size_t t;
                        if(fd)
                        {
                                off = 0;
                                t = vfs_write(fd, REBOOT_RECOVERY_STR, strlen(REBOOT_RECOVERY_STR)+1, &off);
                                printk(KERN_ERR "write to \"/dev/block/platform/mstar_mci.0/by-name/misc\" %ld bytes\n", (long int)t);
                                vfs_fsync(fd, 0);
                                filp_close(fd, NULL);
                        }
                        else
                                printk(KERN_ERR "open \"/dev/block/platform/mstar_mci.0/by-name/misc\" failed\n");
                }
#elif defined(CONFIG_WRITE_TO_ANDROID_MISC_NAND) /* NAND */
# if 1
        if(!Write_Mtd0_init())
                {
            Mtd_erase_ctrl_blocks();
            Write_Mtd0_recovery(REBOOT_RECOVERY_STR,2048);
            //Read_Mtd0_recovery();
        }
        else
        {
            printk(  "Write_Mtd0_init failed\n");
        }
# else
        struct file *fd = filp_open("/dev/mtd/mtd0", O_RDWR, 0);
        char *buff = kzalloc(2048, GFP_USER);
        strncpy(buff, REBOOT_RECOVERY_STR, strlen(REBOOT_RECOVERY_STR)+1);
        if(fd)
        {
            off = 0;
            size_t t = vfs_write(fd, buff, 2048, &off);
            printk(KERN_ERR "write to /dev/mtd/mtd0 %ld bytes\n", t);
            vfs_fsync(fd, 0);
            filp_close(fd, NULL);
        }
        else
            printk(KERN_ERR "open /dev/mtd/mtd0 failed\n");
        kfree(buff);
# endif
#endif
        set_fs(old_fs);
    }
    return NOTIFY_DONE;
}

static struct notifier_block recovery_sys_notifier = {
        .notifier_call    = recovery_sys_notify_reboot,
        .next   = NULL,
        .priority = 0,
};
#endif


unsigned int * uncached_buf = NULL;
static int          dummy_read;
dma_addr_t   buf_phy_addr;

//-----------------------------------------------------------------------------
#ifdef CONFIG_MP_PLATFORM_PIPE_FLUSH_DOUBLE_CHECK
BOOT_MEM_INFO list[2] = { LINUX_MEM2, LINUX_MEM3 };
volatile phys_addr_t alloLXRAM0_PA_BASE, alloLXRAM1_PA_BASE, alloLXRAM2_PA_BASE;
volatile phys_addr_t alloLXRAM0_SIZE, alloLXRAM1_SIZE, alloLXRAM2_SIZE;
volatile phys_addr_t * alloRAM0_VA_BASE, * alloRAM1_VA_BASE, * alloRAM2_VA_BASE;
bool gbInit = false;

#ifdef CONFIG_CMA
extern struct CMA_BootArgs_Config cma_config[MAX_CMA_AREAS];
#endif

#endif
//-----------------------------------------------------------------------------

static int __init Mstar_chip_post_init(void){
#ifdef CONFIG_MP_PLATFORM_PIPE_FLUSH_DOUBLE_CHECK
	//-----------------------------------------------------------------------------
	int i, j, cma_end;
	phys_addr_t tmp_pa, tmp2_pa, tmp_size;

	//! LINUX_MEM
	get_boot_mem_info(LINUX_MEM,  &alloLXRAM0_PA_BASE, &alloLXRAM0_SIZE);
	alloLXRAM1_PA_BASE = alloLXRAM0_PA_BASE;
	alloLXRAM2_PA_BASE = alloLXRAM0_PA_BASE;

	for(i = 0; i < 2; i++)
	{
		get_boot_mem_info(list[i], &tmp_pa, &tmp_size);    //! check the range of LINUX_MEM2

		if (tmp_pa >= ARM_MIU1_BUS_BASE && tmp_pa < ARM_MIU2_BUS_BASE)
		{
			 tmp2_pa = tmp_pa;
		#ifdef CONFIG_CMA
			 for (j = 0; j < MAX_CMA_AREAS; j++)
			 {
			    cma_end = cma_config[j].start + cma_config[j].size;
			    if (cma_config[j].start <= tmp2_pa && cma_end > tmp2_pa)
			    {
				    if (cma_end + 0x1000 <= tmp_pa + tmp_size)
				    {
					    tmp2_pa = cma_end;
						j = 0;
					}
				}
			 }
		#endif
			 tmp_pa = tmp2_pa;
		     if(alloLXRAM1_PA_BASE == alloLXRAM0_PA_BASE)
			 {
	            alloLXRAM1_PA_BASE = tmp_pa;
				alloLXRAM1_SIZE    = tmp_size;
		     }
	    }
		#ifdef  ARM_MIU2_BUS_BASE
		else if (tmp_pa >= ARM_MIU2_BUS_BASE)
		{
			tmp2_pa = tmp_pa;
			#ifdef CONFIG_CMA
			for (j = 0; j < MAX_CMA_AREAS; j++)
			{
			    cma_end = cma_config[j].start + cma_config[j].size;
			    if (cma_config[j].start <= tmp2_pa && cma_end > tmp2_pa)
			    {
				    if (cma_end + 0x1000 <= tmp_pa + tmp_size)
				    {
					    tmp2_pa = cma_end;
						j = 0;
					}
				}
			}
			#endif
			tmp_pa = tmp2_pa;
		    if(alloLXRAM2_PA_BASE == alloLXRAM0_PA_BASE)
			{
				alloLXRAM2_PA_BASE = tmp_pa;
				alloLXRAM2_SIZE    = tmp_size;
		    }
		}
		#endif
	}

	alloRAM0_VA_BASE = (phys_addr_t *)ioremap(alloLXRAM0_PA_BASE, 0x1000); //m <== miu0
	if (alloRAM0_VA_BASE == NULL)
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] remap miu0 address fails. alloLXRAM0_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloLXRAM0_PA_BASE);
	else
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] alloRAM0_VA_BASE: 0x%lx, alloLXRAM0_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloRAM0_VA_BASE, alloLXRAM0_PA_BASE);

	alloRAM1_VA_BASE = (phys_addr_t *)ioremap(alloLXRAM1_PA_BASE, 0x1000);
	if (alloLXRAM1_PA_BASE == alloLXRAM0_PA_BASE)
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] Cannot find miu1 address. alloLXRAM1_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloLXRAM1_PA_BASE);
	else if (alloRAM1_VA_BASE == NULL)
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] remap miu1 address fails. alloLXRAM1_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloLXRAM1_PA_BASE);
	else
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] alloRAM1_VA_BASE: 0x%lx, alloLXRAM1_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloRAM1_VA_BASE, alloLXRAM1_PA_BASE);

#ifdef ARM_MIU2_BUS_BASE
	alloRAM2_VA_BASE = (phys_addr_t *)ioremap(alloLXRAM2_PA_BASE, 0x1000);
	if (alloLXRAM2_PA_BASE == alloLXRAM0_PA_BASE)
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] Cannot find miu2 address. alloLXRAM2_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloLXRAM2_PA_BASE);
	else if (alloRAM2_VA_BASE == NULL)
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] remap miu2 address fails. alloLXRAM2_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloLXRAM2_PA_BASE);
	else
		printk("\033[0;32;31m Function = %s Line = %d, [SAM-CC] alloRAM2_VA_BASE: 0x%lx, alloLXRAM2_PA_BASE: 0x%lx\033[m\n", __func__, __LINE__, alloRAM2_VA_BASE, alloLXRAM2_PA_BASE);
#endif

	gbInit = true;

#ifdef ARM_MIU2_BUS_BASE
	BUG_ON(!alloRAM0_VA_BASE || !alloRAM1_VA_BASE || !alloRAM2_VA_BASE);
#else
	BUG_ON(!alloRAM0_VA_BASE || !alloRAM1_VA_BASE);
#endif

#endif
	//-----------------------------------------------------------------------------

	uncached_buf = arm_dma_alloc(NULL,4 ,&buf_phy_addr, GFP_KERNEL,NULL); 
	printk("\033[0;32;31m Function = %s Line = %d uncached_buf : 0x%lx\033[m\n", __func__, __LINE__, (unsigned long)uncached_buf);
	BUG_ON(!uncached_buf);

        #ifdef CONFIG_WRITE_TO_ANDROID_MISC
        register_reboot_notifier(&recovery_sys_notifier);
        #endif
	return true;
}

postcore_initcall(Mstar_chip_post_init);

/*************************************
*		Mstar chip flush function
*************************************/
#define _BIT(x)                      (1<<(x))
static DEFINE_SPINLOCK(l2prefetch_lock);

inline void _chip_flush_miu_pipe(void)
{
	unsigned long   dwLockFlag = 0;
	unsigned short dwReadData = 0;
    unsigned long ulReadMIU0Data, ulReadMIU1Data;
#ifdef ARM_MIU2_BUS_BASE
	unsigned long ulReadMIU2Data;
#endif
	spin_lock_irqsave(&l2prefetch_lock, dwLockFlag);

    //toggle the flush miu pipe fire bit, 0x10188a[0
    *(volatile unsigned short *)(mstar_pm_base + (0x203114)) &= ~(0x0001);
    *(volatile unsigned short *)(mstar_pm_base + (0x203114)) |= 0x0001; 

	// check bits are 0x1018a1[4]~ 0x1018a1[6]
	do
	{
    	dwReadData = *(volatile unsigned short *)(mstar_pm_base + (0x203140)); 
        dwReadData &= (_BIT(12));  //Check Status of Flush Pipe Finish

    } while(dwReadData != (_BIT(12)));	// we need to check 1 bits 0x1018a1[4] miu0 & miu1 all flush done
    #ifdef CONFIG_MP_PLATFORM_PIPE_FLUSH_DOUBLE_CHECK
	//! dummy read
	if (gbInit == true)
	{
		ulReadMIU0Data = (unsigned long)(* alloRAM0_VA_BASE);
		ulReadMIU1Data = (unsigned long)(* alloRAM1_VA_BASE);
#ifdef ARM_MIU2_BUS_BASE
		ulReadMIU2Data = (unsigned long)(* alloRAM2_VA_BASE);
#endif
	}
#endif
	spin_unlock_irqrestore(&l2prefetch_lock, dwLockFlag);
}
void Chip_Flush_Miu_Pipe(void)
{
    _chip_flush_miu_pipe();
}
EXPORT_SYMBOL(Chip_Flush_Miu_Pipe);

//add for fixing compile warning
void CPU_Cache_Flush_All(void *p)
{
	__cpuc_flush_kern_all();
}

void Chip_Flush_Cache_All(void)
{
	unsigned long flags;

	local_irq_save(flags);

	//Clean & Inv All L1
	__cpuc_flush_kern_all();

	local_irq_restore(flags);

	smp_call_function(CPU_Cache_Flush_All, NULL, 1);

	local_irq_save(flags);

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Clean&Inv L2 by Way
		outer_cache.flush_all();
	}
#endif
	//Clean L1  & Inv L1
	//dmac_flush_range(u32Addr,u32Addr + u32Size );

#ifndef CONFIG_OUTER_CACHE
	_chip_flush_miu_pipe();
#endif

	local_irq_restore(flags);
}
EXPORT_SYMBOL(Chip_Flush_Cache_All);

void Chip_Flush_Cache_Range(unsigned long u32Addr, unsigned long u32Size)
{
	if(  u32Addr == (unsigned long) NULL )
        {
                printk("u32Addr is invalid\n");
                return;
        }
	//Clean L1 & Inv L1
	dmac_flush_range((const void*)u32Addr,(const void*)(u32Addr + u32Size));

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		if(!virt_addr_valid(u32Addr) || !virt_addr_valid(u32Addr+ u32Size - 1))
			//Clean&Inv L2 by Way
			outer_cache.flush_all();
		else
			//Clean&Inv L2 by Range
			outer_cache.flush_range(__pa(u32Addr) ,__pa(u32Addr)+ u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE //flush miu pipe for L2 disabled case
	_chip_flush_miu_pipe();
#endif
}
EXPORT_SYMBOL(Chip_Flush_Cache_Range);

void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size)
{
        if(  u32VAddr == (unsigned long) NULL )
        {
                printk("u32VAddr is invalid\n");
                return;
        }
	//Clean & Invalid L1
	dmac_flush_range((const void*)u32VAddr,(const void*)(u32VAddr + u32Size));

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Clean&Inv L2 by range
		outer_cache.flush_range(u32PAddr,u32PAddr + u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE
	_chip_flush_miu_pipe();
#endif
}
EXPORT_SYMBOL(Chip_Flush_Cache_Range_VA_PA);

void Chip_Flush_Memory(void)
{
    #ifdef CONFIG_OUTER_CACHE
	if(outer_cache.sync)
		outer_cache.sync();
	else
	#endif
    {
		extern ptrdiff_t mstar_pm_base;
        //flush store buffer
        *uncached_buf= *(volatile unsigned short *)(mstar_pm_base + (0x101274 << 1));
        smp_mb();
        dummy_read = *uncached_buf;
		_chip_flush_miu_pipe();
	}
}
EXPORT_SYMBOL(Chip_Flush_Memory);

void Chip_Inv_Cache_Range(unsigned long u32Addr, unsigned long u32Size)
{
        if(  u32Addr == (unsigned long) NULL )
        {
                printk("u32Addr is invalid\n");
                return;
        }
#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{

		if(!virt_addr_valid(u32Addr) || !virt_addr_valid(u32Addr+ u32Size - 1))
			printk(KERN_DEBUG "Input VA can't be converted to PA\n");
		else
			//Inv L2 by range
			outer_cache.inv_range(__pa(u32Addr) , __pa(u32Addr) + u32Size);
	}
#endif
	//Inv L1
	dmac_map_area((const void *)u32Addr,(size_t)u32Size,2);
}
EXPORT_SYMBOL(Chip_Inv_Cache_Range);

void Chip_Flush_Cache_All_Single(void)
{
	unsigned long flags;

	local_irq_save(flags);

	//Clean & Inv All L1
	__cpuc_flush_kern_all();

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()&&outer_cache.flush_all) //check if L2 is enabled
	{
		//Clean&Inv L2 by Way
		outer_cache.flush_all();
	}
#endif
	//Clean L1  & Inv L1
	//dmac_flush_range(u32Addr,u32Addr + u32Size );

#ifndef CONFIG_OUTER_CACHE
	_chip_flush_miu_pipe();
#endif

	local_irq_restore(flags);
}
EXPORT_SYMBOL(Chip_Flush_Cache_All_Single);

void Chip_Clean_Cache_Range(unsigned long u32Addr, unsigned long u32Size)
{
        if(  u32Addr == (unsigned long) NULL )
        {
                printk("u32Addr is invalid\n");
                return;
        }
	//Clean L1
	dmac_map_area((const void *)u32Addr,(size_t)u32Size,1);

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		if(!virt_addr_valid(u32Addr) || !virt_addr_valid(u32Addr+ u32Size - 1))
			//Clean L2 by Way
			outer_cache.clean_all();
		else
			//Clean L2 by Range
			outer_cache.clean_range( __pa(u32Addr),__pa(u32Addr) + u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE //flush miu pipe for L2 disabled case
	_chip_flush_miu_pipe();
#endif
}
EXPORT_SYMBOL(Chip_Clean_Cache_Range);

//need to be modified
void Chip_L2_cache_wback_inv( unsigned long addr, unsigned long size)
{
#ifdef CONFIG_OUTER_CACHE //flush miu pipe for L2 disabled case
	outer_cache.flush_all();
#endif
}
EXPORT_SYMBOL(Chip_L2_cache_wback_inv);

//need to be modified
void Chip_L2_cache_wback( unsigned long addr, unsigned long size)
{
	//Flush L2 by Way, change to by Addr later
#ifdef CONFIG_OUTER_CACHE //flush miu pipe for L2 disabled case
	outer_cache.clean_all();
#endif
}
EXPORT_SYMBOL(Chip_L2_cache_wback);

//need to be modified
void Chip_L2_cache_inv( unsigned long addr, unsigned long size)
{
	//Inv L2 by Way, change to by Addr later
#ifdef CONFIG_OUTER_CACHE //flush miu pipe for L2 disabled case
	outer_cache.inv_all();
#endif
}
EXPORT_SYMBOL(Chip_L2_cache_inv);

void Chip_Flush_Memory_Range(unsigned long pAddress , unsigned long  size)
{
    #ifdef CONFIG_OUTER_CACHE
	if(outer_cache.sync)
		outer_cache.sync();
	else
    #endif
    {
        //flush store buffer
        *uncached_buf= *(volatile unsigned short *)(mstar_pm_base + (0x101274 << 1));
        smp_mb();
        dummy_read = *uncached_buf;
		_chip_flush_miu_pipe();
	}
}
EXPORT_SYMBOL(Chip_Flush_Memory_Range);

void Chip_Read_Memory(void)
{
    #ifdef CONFIG_OUTER_CACHE
	if(outer_cache.sync)
		outer_cache.sync();
	else
	#endif
    {
        //flush store buffer
        *uncached_buf= *(volatile unsigned short *)(mstar_pm_base + (0x101274 << 1));
        smp_mb();
        dummy_read = *uncached_buf;
		_chip_flush_miu_pipe();
	}
}
EXPORT_SYMBOL(Chip_Read_Memory);

void Chip_Read_Memory_Range(unsigned long pAddress , unsigned long  size)
{
    #ifdef CONFIG_OUTER_CACHE
	if(outer_cache.sync)
		outer_cache.sync();
	else
	#endif
    {
        //flush store buffer
        *uncached_buf= *(volatile unsigned short *)(mstar_pm_base + (0x101274 << 1));
        smp_mb();
        dummy_read = *uncached_buf;
		_chip_flush_miu_pipe();
	}
}
EXPORT_SYMBOL(Chip_Read_Memory_Range);

void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size)
{
        if(  u32VAddr == (unsigned long) NULL )
        {
                printk("u32VAddr is invalid\n");
                return;
        }
	//Clean L1
	dmac_map_area((const void *)u32VAddr,(size_t)u32Size,1);

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Clean L2 by Way
		outer_cache.clean_range(u32PAddr,u32PAddr + u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE
	_chip_flush_miu_pipe();
#endif
}
EXPORT_SYMBOL(Chip_Clean_Cache_Range_VA_PA);

void Chip_Inv_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size)
{
        if(  u32VAddr == (unsigned long) NULL )
        {
                printk("u32VAddr is invalid\n");
                return;
        }

#ifdef CONFIG_OUTER_CACHE
	if (outer_cache.is_enable()) //check if L2 is enabled
	{
		//Inv L2 by range
		outer_cache.inv_range( u32PAddr ,u32PAddr + u32Size );
	}
#endif
	//Inv L1
	dmac_map_area((const void *)u32VAddr,(size_t)u32Size,2);
}
EXPORT_SYMBOL(Chip_Inv_Cache_Range_VA_PA);

unsigned int query_frequency(void)
{
#ifdef CONFIG_MSTAR_ARM_BD_FPGA
    return 12;
#else
	unsigned short freq_type = reg_readw(0x1F000000 + 0x101D00 * 2 + 0xF8 * 2);
	unsigned short freq_type1 = reg_readw(0x1F000000 + 0x101D00 * 2 + 0xFA * 2);
	
	unsigned short freq_low, freq_high;

	if((freq_type&0x1)==1)
	{
		freq_low = reg_readw(0x1F000000 + 0x110C00 * 2 + 0x60 * 4);
		freq_high = reg_readw(0x1F000000 + 0x110C00 * 2 + 0x61 * 4);
		printk("\033[0;32;31m [Kernel] %s %d frequency : %ldM \033[m\n",__func__,__LINE__,(3623878000/ ((freq_high << 16) + freq_low)));
		return (3623878000/ ((freq_high << 16) + freq_low));
	}
	else if((freq_type1&0x1)==1)
	{
		printk("\033[0;32;31m [Kernel] %s %d frequency : 216M \033[m\n",__func__,__LINE__);
		return 216;
	}
	else
	{
		printk("\033[0;32;31m [Kernel] %s %d frequency : 12M \033[m\n",__func__,__LINE__);
		return 12;
	}
#endif
}
EXPORT_SYMBOL(query_frequency);

#ifdef CONFIG_WRITE_TO_ANDROID_MISC_NAND
static struct mtd_info * ctrl_mtd;
static __u8 u8_Partfound;

//extern struct mtd_info * __init open_mtd_device(const char *mtd_dev);

extern struct mtd_info *get_mtd_device_nm(const char *name);
extern struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num);
/**
 * open_mtd_by_chdev - open an MTD device by its character device node path.
 * @mtd_dev: MTD character device node path
 *
 * This helper function opens an MTD device by its character node device path.
 * Returns MTD device description object in case of success and a negative
 * error code in case of failure.
 */
static struct mtd_info * __init open_mtd_by_chdev(const char *mtd_dev)
{
        int err, major, minor, mode;
        struct path path;

        /* Probably this is an MTD character device node path */
        err = kern_path(mtd_dev, LOOKUP_FOLLOW, &path);
        if (err)
                return ERR_PTR(err);

        /* MTD device number is defined by the major / minor numbers */
        major = imajor(path.dentry->d_inode);
        minor = iminor(path.dentry->d_inode);
        mode = path.dentry->d_inode->i_mode;
        path_put(&path);
        if (major != MTD_CHAR_MAJOR || !S_ISCHR(mode))
                return ERR_PTR(-EINVAL);

        if (minor & 1)
                /*
                 * Just do not think the "/dev/mtdrX" devices support is need,
                 * so do not support them to avoid doing extra work.
                 */

                return ERR_PTR(-EINVAL);

        return get_mtd_device(NULL, minor / 2);
}



static struct mtd_info * __init open_mtd_device(const char *mtd_dev)
{
        struct mtd_info *mtd;
        int mtd_num;
        char *endp;

        mtd_num = simple_strtoul(mtd_dev, &endp, 0);
        if (*endp != '\0' || mtd_dev == endp) {
                /*
                 * This does not look like an ASCII integer, probably this is
                 * MTD device name.
                 */
                mtd = get_mtd_device_nm(mtd_dev);
                if (IS_ERR(mtd) && PTR_ERR(mtd) == -ENODEV)
                        /* Probably this is an MTD character device node path */
                        mtd = open_mtd_by_chdev(mtd_dev);
        } else
                mtd = get_mtd_device(NULL, mtd_num);

        return mtd;
}


int Write_Mtd0_init(void)
{
        int err = 0;
        u8_Partfound = 0;
        ctrl_mtd = open_mtd_device("misc");
        if(IS_ERR(ctrl_mtd))
        {
                printk("Can't find \"misc\" partition\n");
                err = PTR_ERR(ctrl_mtd);
                return err;
        }
        u8_Partfound = 1;

        return err;
}



#define ENINIT_MAX_SQNUM                        0x7FFFFFFF
//control error code
#define ERR_CTRL_RECORD_FOUND           (0x600|1)
#define ERR_CTRL_RECORD_INVALID         (0x600|2)
#define ERR_CTRL_RECORD_FAIL            (0x600|3)
#define ERR_CTRL_RECORD_NOFOUND         (0x600|4)
#define ERR_CTRL_RECORD_NOAVLBLK        (0x600|5)
#define ERR_CTRL_RECORD_WRITEFAIL       (0x600|6)
#define ERR_TBL_RECORD_POWERCUT         (0x600|7)
#define ERR_TBL_RECORD_NOAVLBLK         (0x600|8)


int Mtd_erase_ctrl_blocks(void)
{
        int blk_page_count, err;
        __u32 u32_i, cb_count;
        struct erase_info ei;
        blk_page_count = (ctrl_mtd->erasesize/ctrl_mtd->writesize);
_count = ctrl_mtd->size >> ctrl_mtd->writesize_shift;

        for(u32_i = 0; u32_i < cb_count; u32_i += blk_page_count)
        {
                err = ctrl_mtd->block_isbad(ctrl_mtd, u32_i << ctrl_mtd->writesize_shift);
                if(err == 1)
                {
                        printk("Skip bad block %X", u32_i);
                        continue;
                }
                else if (err < 0)
                {
                        return ERR_CTRL_RECORD_FAIL;
                }

                memset(&ei, 0, sizeof(struct erase_info));
                ei.mtd = ctrl_mtd;
                ei.addr = (u32_i / blk_page_count) << ctrl_mtd->erasesize_shift;
                ei.len = ctrl_mtd->erasesize;
                ei.callback = NULL;

                err = ctrl_mtd->erase(ctrl_mtd, &ei);
                if(err == -EIO)
                {
                        err = ctrl_mtd->block_markbad(ctrl_mtd,
                                        (u32_i / blk_page_count)<< ctrl_mtd->erasesize_shift);
                        if(err < 0)
                                return err;
                }
                else if(err < 0 && err != -EIO)
                {
                        return err;
                }

        }
        return 0;
}

int Write_Mtd0_recovery(__u8* DataBuf,unsigned long u32Size)
{
    __u8* pu8_DataBuf;
    __u32 err = 0;
        size_t written;
    pu8_DataBuf = kzalloc(ctrl_mtd->writesize, GFP_KERNEL);
        memcpy(pu8_DataBuf,DataBuf,u32Size);
        if(!pu8_DataBuf)
                return err;

    err = ctrl_mtd->write(ctrl_mtd, 0, ctrl_mtd->writesize, &written, pu8_DataBuf);

    kfree(pu8_DataBuf);
        return err;
}

int Read_Mtd0_recovery(void)
{
    __u8* pu8_DataBuf;
    __u32 err = 0;
    size_t read;
        pu8_DataBuf = kzalloc(ctrl_mtd->writesize, GFP_KERNEL);
        memset(pu8_DataBuf, 0xFF, ctrl_mtd->writesize);
    err = ctrl_mtd->read(ctrl_mtd,0, 10, &read, pu8_DataBuf);

        printk("%d %d %d \n",pu8_DataBuf[0],pu8_DataBuf[1],pu8_DataBuf[2]);
        return err;
}

#endif

