
#include <asm/arch/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/timing.h>
#include <asm/arch/reg_addr.h>
#define DDR_RSLR_LEN 3
#define DDR_RDGR_LEN 4
#define DDR_MAX_WIN_LEN (DDR_RSLR_LEN*DDR_RDGR_LEN)
#define PHYS_MEMORY_START 0x80000000

#define DDR_RANK  3   // 2'b11 means 2 rank.
#define DDR_LANE  2

#define RESET_MMC_FOR_DTU_TEST

#define NOP_CMD  0
#define PREA_CMD  1
#define REF_CMD  2
#define MRS_CMD  3
#define ZQ_SHORT_CMD 4
#define ZQ_LONG_CMD  5
#define SFT_RESET_CMD 6


static unsigned ddr_start_again=1;

#define APB_Wr(addr, data) WRITE_APB_REG(addr,data)
#define APB_Rd(addr) READ_APB_REG(addr)
#define Wr           WRITE_CBUS_REG
#define Rd           READ_CBUS_REG

//#include "ddr_init_sw.c"
#include "ddr_init_hw.c"
#include "ddr_init_pctl.c"

static void wait_pll(unsigned clk,unsigned dest);

void set_ddr_clock(struct ddr_set * timing_reg)
{
	/*
#ifdef ENABLE_POWER_SAVING
    APB_Wr(PCTL_DLLCR_ADDR, APB_Rd(PCTL_DLLCR_ADDR)|(7<<2));
#endif
	*/
	int pll_times=0;

	do{
		M6_PLL_RESET(HHI_DDR_PLL_CNTL);
		Wr(HHI_DDR_PLL_CNTL2,M6_DDR_PLL_CNTL_2);
		Wr(HHI_DDR_PLL_CNTL3,M6_DDR_PLL_CNTL_3);
		Wr(HHI_DDR_PLL_CNTL4,M6_DDR_PLL_CNTL_4);
#ifdef CONFIG_CMD_DDR_TEST
		if((Rd(PREG_STICKY_REG0) & 0xffff) == 0x2012){
	        zqcr = (Rd(PREG_STICKY_REG0) >> 16);
			Wr(HHI_DDR_PLL_CNTL, Rd(PREG_STICKY_REG1));
			Wr(PREG_STICKY_REG0, 0);
	        Wr(PREG_STICKY_REG1, 0);
		}
		else
#endif
		Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl); //board/xxx/firmware/timming.c
		M6_PLL_LOCK_CHECK(pll_times,5);
	}while(M6_PLL_IS_NOT_LOCK(HHI_DDR_PLL_CNTL));

	/*
	#define MMC_CLK_CNTL      0xc800641c
	//bit 31     1 disabel all clock.
	//bit 30.    1 enable  auto clock gating. 0 : enable all clock if bit 31 = 0;
	//bit 29.    DDR PLL lock signal.   DDR PLL locked = 1.  
	//bit  7.    dll_clk_sel. 1 : the DLL input is directly from DDR PLL.  0: the DLL input is from slow clock or from DDR PLL clock divider. 
	//bit  6.    pll_clk_en.  1 : enable the DDR PLL clock to DDR DLL path. 0 : disable the DDR PLL clock to DDR PLL path.
	//bit  5.    divider/slow clock selection.   1 = slow clock.  0 = DDR PLL clock divider.  
	//bit  4.    slow clock enable.     1 = slow clock en.  0 : disalbe slow clock.
	//bit  3.    PLL clk divider enable. 1 = enable. 0 disable.
	//bit  2.    divider clock output enalbe.
	//bit 1:0    divider:    00 : /2.   01: /4. 10 : /8. 11: /16. 

	*/

  	//Enable DDR DLL clock input from PLL.
    writel(0xc0000080, P_MMC_CLK_CNTL);  //  @@@ select the final mux from PLL output directly.
    writel(0xc00000c0, P_MMC_CLK_CNTL);

    //enable the clock.
    writel(0x400000c0, P_MMC_CLK_CNTL);
     
    // release the DDR DLL reset pin.
    writel(0xffff, P_MMC_SOFT_RST);
#ifndef CONFIG_CMD_DDR_TEST
	wait_pll(3,timing_reg->ddr_clk);
#endif
    //serial_puts("set ddr clock ok!\n");
#if !defined(CONFIG_M6_NOT_DUMP_DDR_INFO)

	if(!((readl(0xc1107d54) >> 9) & 1))	
	{
		//error, because all clk setting base on 24MHz
		serial_puts("\nERROR! XTAL is wrong, try to reset...\n");
		__udelay(10000); 
		AML_WATCH_DOG_START();
	}

	int nPLL = readl(P_HHI_DDR_PLL_CNTL);
	int nDDRCLK = ((24 / ((nPLL>>9)& 0x1F) ) * (nPLL & 0x1FF))/ (1<<((nPLL>>16) & 0x3));
	serial_puts("\nDDR clock is ");
	serial_put_dec(nDDRCLK);
	serial_puts("MHz with ");
	
	#ifdef CONFIG_DDR_LOW_POWER
	serial_puts("Low Power & ");
	#endif
	
	if(timing_reg->mcfg & (1<<3))
		serial_puts("2T mode\n");
	else
		serial_puts("1T mode\n");
#else
	serial_puts("\nset ddr clock ok!\n");
#endif

	serial_puts("\nDDR training :");

}

#ifdef DDR_ADDRESS_TEST_FOR_DEBUG
static void ddr_addr_test()
{
    unsigned i, j = 0;

addr_start:
    serial_puts("********************\n");
    writel(0x55aaaa55, PHYS_MEMORY_START);
    for(i=2;(((1<<i)&PHYS_MEMORY_SIZE) == 0) ;i++)
    {
        writel((i-1)<<(j*8), PHYS_MEMORY_START+(1<<i));
    }

    serial_put_hex(PHYS_MEMORY_START, 32);
    serial_puts(" = ");
    serial_put_hex(readl(PHYS_MEMORY_START), 32);
    serial_puts("\n");
    for(i=2;(((1<<i)&PHYS_MEMORY_SIZE) == 0) ;i++)
    {
       serial_put_hex(PHYS_MEMORY_START+(1<<i), 32);
       serial_puts(" = ");
       serial_put_hex(readl(PHYS_MEMORY_START+(1<<i)), 32);
       serial_puts("\n");
    }

    if(j < 3){
        ++j;
        goto addr_start;
    }
}
#endif

static inline unsigned lowlevel_ddr(unsigned tag,struct ddr_set * timing_reg)
{
    set_ddr_clock(timing_reg);
    //if(tag)
    //    return ddr_init_sw(timing_reg);
    return ddr_init_hw(timing_reg);
}
static inline unsigned lowlevel_mem_test_device(unsigned tag,struct ddr_set * timing_reg)
{
    return tag&&(unsigned)memTestDevice((volatile datum *)PHYS_MEMORY_START,PHYS_MEMORY_SIZE);
}

static inline unsigned lowlevel_mem_test_data(unsigned tag,struct ddr_set * timing_reg)
{
    return tag&&(unsigned)memTestDataBus((volatile datum *) PHYS_MEMORY_START);
}

static inline unsigned lowlevel_mem_test_addr(unsigned tag,struct ddr_set * timing_reg)
{
	//asm volatile ("wfi");
#ifdef DDR_ADDRESS_TEST_FOR_DEBUG
    ddr_addr_test();
    return 0;
#else
    return tag&&(unsigned)memTestAddressBus((volatile datum *)PHYS_MEMORY_START, PHYS_MEMORY_SIZE);
#endif
}

static unsigned ( * mem_test[])(unsigned tag,struct ddr_set * timing_reg)={
    lowlevel_ddr,
    lowlevel_mem_test_addr,
    lowlevel_mem_test_data,
#ifdef CONFIG_ENABLE_MEM_DEVICE_TEST
    lowlevel_mem_test_device
#endif
};

#define MEM_DEVICE_TEST_ITEMS_BASE (sizeof(mem_test)/sizeof(mem_test[0]))

unsigned m6_ddr_init_test(int arg)
{
    int i; //,j;
    unsigned por_cfg=1;
    serial_putc('\n');
    por_cfg=0;
    for(i=0;i<MEM_DEVICE_TEST_ITEMS_BASE&&por_cfg==0;i++)
	{
        writel(0, P_WATCHDOG_RESET);
        por_cfg=mem_test[i](arg&(1<<i),&__ddr_setting);
        serial_puts("\nStage ");
        serial_put_hex(i,8);
        serial_puts(" Result ");
        serial_put_hex(por_cfg,32);
	}
	writel(0, P_WATCHDOG_TC);
	ddr_start_again=por_cfg?1:ddr_start_again;
	return por_cfg;
}
SPL_STATIC_FUNC unsigned ddr_init_test(void)
{
#define DDR_INIT_START  (1)
#define DDR_TEST_ADDR   (1<<1)
#define DDR_TEST_DATA   (1<<2)
#define DDR_TEST_DEVICE (1<<3)

#define DDR_TEST_BASEIC (DDR_INIT_START|DDR_TEST_ADDR|DDR_TEST_DATA)
#define DDR_TEST_ALL    (DDR_TEST_BASEIC|DDR_TEST_DEVICE)


	if(m6_ddr_init_test(DDR_TEST_BASEIC))
    {
	    serial_puts("\nDDR init test fail! Reset...\n");
		__udelay(10000); 
		AML_WATCH_DOG_START();
	}

	return 0;
}
	
