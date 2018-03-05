
#include <mstar/mstar_chip.h>
#include "mdrv_mma_heap.h"

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/swap.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/mman.h>
#include <linux/export.h>
#include <linux/nodemask.h>
#include <linux/initrd.h>
#include <linux/of_fdt.h>
#include <linux/highmem.h>
#include <linux/gfp.h>
#include <linux/memblock.h>
#include <linux/dma-contiguous.h>
#include <linux/sizes.h>

#include <asm/mach-types.h>
#include <asm/memblock.h>
#include <asm/prom.h>
#include <asm/sections.h>
#include <asm/setup.h>
#include <asm/tlb.h>
#include <asm/fixmap.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

int mstar_driver_boot_mma_buffer_num = 0;
struct MMA_BootArgs_Config mma_config[MAX_MMA_AREAS];

EXPORT_SYMBOL(mma_config);
EXPORT_SYMBOL(mstar_driver_boot_mma_buffer_num);

/*
example:
mma_heap=mma_heap_name0,miu=0,sz=0x300000 
mma_heap=mma_heap_name_1,miu=0,sz=0x200000
mma_heap=mma_heap_name_2,miu=0,sz=0x400000 
mma_heap=mma_heap_name_3,miu=1,sz=0x100000
*/
static bool parse_mmaheap_config(char *cmdline, struct MMA_BootArgs_Config * heapconfig)
{
    char *option;
    int leng = 0;
    bool has_start = false;
    int i;

    if(cmdline == NULL)
        goto INVALID_HEAP_CONFIG;

    option = strstr(cmdline, ",");
    leng = (int)(option - cmdline);
    if(leng > (MMA_HEAP_NAME_LENG-1))
        leng = MMA_HEAP_NAME_LENG -1;

    strncpy(heapconfig->name, cmdline, leng);
    heapconfig->name[leng] = '\0';

    option = strstr(cmdline, "sz=");
    if(option == NULL)
        goto INVALID_HEAP_CONFIG;

    option = strstr(cmdline, "miu=");
    if(option == NULL)
        goto INVALID_HEAP_CONFIG;

   sscanf(option, "miu=%d,sz=%lx", &heapconfig->miu, &heapconfig->size);

#if 0//debug code
    printk(" debug %s %d  success name:",__FUNCTION__,__LINE__);
        for(i=0;i<leng;i++)
            {
            printk("%c ",heapconfig->name[i]);
            }
        printk("    miu=%d,sz=%lx \n", heapconfig->miu, heapconfig->size);
#endif
        
    return true;

INVALID_HEAP_CONFIG:
	heapconfig->size = 0;
       #if 0//debug code
       printk("samson debug %s %d  fail \n",__FUNCTION__,__LINE__);
       #endif
	return false;
}
int __init setup_mma_info(char *cmdline)
{
    if(!parse_mmaheap_config(cmdline, &mma_config[mstar_driver_boot_mma_buffer_num]))        
        printk(KERN_ERR "error: mma_heap args invalid\n");
    else
        mstar_driver_boot_mma_buffer_num++;

    return 0;
}

early_param("mma_heap", setup_mma_info);

extern unsigned long lx_mem_size;// = INVALID_PHY_ADDR;
extern unsigned long lx_mem2_size;// = INVALID_PHY_ADDR;
extern unsigned long lx_mem3_size;// = INVALID_PHY_ADDR;

extern unsigned long lx_mem_addr;
extern unsigned long lx_mem2_addr;
extern unsigned long lx_mem3_addr;

void deal_with_reserve_mma_heap()
{
    int i=0,j=0,k=0;
    phys_addr_t base;
    int ret;
    phys_addr_t start;
    phys_addr_t end;
    phys_addr_t size;

    if(mstar_driver_boot_mma_buffer_num == 0)
    {
         //need do nothing
         printk(KERN_WARNING "no any mma heap\n");
        return;
    }

    //check whether have same mma heap name.
    for(i=0;i<mstar_driver_boot_mma_buffer_num - 1;i++)
    {
          if(mma_config[i].size == 0)
          {
             printk(KERN_WARNING "mma heap size is 0, skip\n");
             continue;
          }
          for(j=i+1;j<mstar_driver_boot_mma_buffer_num;j++)
          {
              if(mma_config[j].size == 0)
              {
                  printk(KERN_WARNING "mma heap size is 0, skip\n");
                  continue;
              }
              if(0 == strncmp((char *)(&mma_config[i].name[0]),(char *)(&mma_config[j].name[0]),strlen((char *)(&mma_config[i].name[0]))+1))//use strlen() +1
              {
                  printk("fail,error  ,find same mma heap!!!! \n");
                  printk("Please do not use different mma heap with same name , though they may in different miu or have different size\n");

                  printk("same mma heap name:  ");
                  for(k=0;k<strlen((char *)(&mma_config[i].name[0]));k++)
                  {
                    printk("%c",mma_config[i].name[k]);
                  }
                  printk("\n");


                  /*do not give BUG(),for current function deal_with_reserve_mma_heap will
                  be called by arm_memblock_init,and it is in early stage of starting kernel.
                  if BUG() while cause log only be
                  "
                  Starting kernel ...

                  Uncompressing Linux... done, booting the kernel.
                  "
                  and cause not easy to find what happened!!

                  here replace BUG() with much normal printk to let user notice what happened for debug easier.
                  */
                  //BUG();
                  printk("#####################################\n");
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("%s:%d  fail,error  ,find same mma heap!!!! \n",__FUNCTION__,__LINE__);
                  printk("#####################################\n");

                  printk("%s:%d  fail,error  return directly!!!!!!!!!!!!!!\n",__FUNCTION__,__LINE__);
                  return;
              }
          }
    }

    for(i=0;i<mstar_driver_boot_mma_buffer_num;i++)
    {
        mma_config[i].reserved_start  =  INVALID_PHY_ADDR;//init with invalid phy addr
        size = mma_config[i].size;
        if(size == 0)
        {
            printk(KERN_WARNING "mma heap size is 0, skip\n");
            continue;
        }
        switch(mma_config[i].miu)
        {
            case 0://miu0
                start = lx_mem_addr;
                end = lx_mem_addr + lx_mem_size;
                break;
            case 1://miu1
                start = lx_mem2_addr;
                end = lx_mem2_addr + lx_mem2_size;
                break;
            case 2://miu2
                start = lx_mem3_addr;
                end = lx_mem3_addr + lx_mem3_size;
                break;
            default:
                printk(KERN_ERR "error!!! %s:%d not support miu %d in this chip!!!!\n",__FUNCTION__,__LINE__,mma_config[i].miu);
                return;
                break;
        }

        base = memblock_find_in_range(start, end, size, PAGE_SIZE);
        if(base == 0)//fail
        {
            printk(KERN_ERR "memblock_find_in_range fail ");
            #ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("start=0x%llx, end=0x%llx, size=0x%llx\n",start, end, size);
            #else
            printk("start=0x%lx, end=0x%lx, size=0x%lx\n",start, end, size);
            #endif



            //In early stage of starting kernel,do not use BUG().
            //replace BUG() with much normal printk to let user notice what happened for debug easier.
            //BUG();
            printk("#####################################\n");
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("#####################################\n");

            return;
        }

        //In early stage of starting kernel,do not use BUG_ON().
        //replace BUG_ON() with much normal printk to let user notice what happened for debug easier.
        //BUG_ON(base < start || base > end);
        if(base < start || base > end)
        {
            printk("#####################################\n");
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d base range  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("#####################################\n");
            return;
        }

        //printk("memblock_find_in_range success   \n");

        ret = memblock_reserve(base,size);
        //printk("%s:%d ",__FUNCTION__,__LINE__);
        if(ret != 0)//fail
        {
            printk(KERN_ERR "%s memblock_reserve fail ,  ret=%d\n",__FUNCTION__,ret);
            #ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("baset=0x%llx   size=0x%llx\n",base,size);
            #else
             printk("base=0x%lx   size=0x%lx\n",base,size);
            #endif

            //In early stage of starting kernel,do not use BUG().
            //replace BUG() with much normal printk to let user notice what happened for debug easier.
            //BUG();
            printk("#####################################\n");
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n",__FUNCTION__,__LINE__);
            printk("#####################################\n");
            return;
        }
        else
        {
            mma_config[i].reserved_start  =  base;
            printk("%s memblock_reserve success mma_config[%d].reserved_start=",__FUNCTION__,i);
            #ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("0x%llx",mma_config[i].reserved_start);
            #else
            printk("0x%lx",mma_config[i].reserved_start);
            #endif

#if 0//debug code
    printk(" debug %s %d   name:",__FUNCTION__,__LINE__);
        for(j=0;j<strlen(mma_config[i].name);j++)
        {
            printk("%c ",mma_config[i].name[j]);
        }
        printk("    miu=%d,sz=%lx ", mma_config[i].miu, mma_config[i].size);
        #ifdef CONFIG_PHYS_ADDR_T_64BIT
        printk("reserved_start=0x%llx\n",mma_config[i].reserved_start);
        #else
        printk("reserved_start=0x%lx\n",mma_config[i].reserved_start);
        #endif
#endif

        }
    }
}




