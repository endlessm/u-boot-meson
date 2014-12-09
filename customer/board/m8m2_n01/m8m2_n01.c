#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <malloc.h>

#if defined(CONFIG_CMD_NET)
#include <asm/arch/aml_eth_reg.h>
#include <asm/arch/aml_eth_pinmux.h>
#include <asm/arch/io.h>
#endif /*(CONFIG_CMD_NET)*/

#if defined(CONFIG_AML_I2C)
#include <aml_i2c.h>
#include <asm/arch/io.h>
#endif /*CONFIG_AML_I2C*/

#ifdef CONFIG_PLATFORM_HAS_PMU
#include <amlogic/aml_pmu_common.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

u32 get_board_rev(void)
{
 
	return 0x20;
}

#if CONFIG_AML_HDMI_TX
/*
 * Init hdmi related power configuration
 * Refer to your board SCH, power including HDMI5V, HDMI1.8V, AVDD18_HPLL, etc
 */
extern void hdmi_tx_power_init(void);
void hdmi_tx_power_init(void)
{
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_9, 1 << 0);//Clear Y16 pinmux
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_3, 1 << 5);//clear Y16 pinmux
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, 1 << 29);//clear Y16 pinmux
    CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO1_EN_N, 1 << 16); //Set Y16 to output
    SET_CBUS_REG_MASK(PREG_PAD_GPIO1_O, 1 << 16);//set Y16 to high level
    printf("hdmi tx power init\n");
}
#endif

#if CONFIG_CMD_MMC
#include <mmc.h>
#include <asm/arch/sdio.h>
static int  sdio_init(unsigned port)
{
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            //todo add card detect 	
            setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
            break;
        case SDIO_PORT_C:    	
            //enable pull up
            clrbits_le32(P_PAD_PULL_UP_REG3, 0xff<<0);
            break;
        case SDIO_PORT_XC_A:
            break;
        case SDIO_PORT_XC_B:
            break;
        case SDIO_PORT_XC_C:
            break;
        default:
            break;
    }

    return cpu_sdio_init(port);
}

extern unsigned sdio_debug_1bit_flag;

static int  sdio_detect(unsigned port)
{	
    int ret;
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
            ret=readl(P_PREG_PAD_GPIO5_I)&(1<<29)?0:1;
            printf( " %s return %d\n",__func__,ret);
            
			if(!(readl(P_PREG_PAD_GPIO0_I)&(1<<26))){ //sd_d3 low, debug board in
				if(!(readl(P_PREG_PAD_GPIO0_I)&(1<<22))){
					printf("sdio debug board detected, sd card with 1bit mode\n");
		 			sdio_debug_1bit_flag = 1;
		 		}
		 		else{ 
		 			printf("sdio debug board detected, no sd card in\n");
		 			sdio_debug_1bit_flag = 0;
		 			return 1;
		 		}
		 	}
		 	          
            break;
        case SDIO_PORT_C:    	
            break;
        case SDIO_PORT_XC_A:
            break;
        case SDIO_PORT_XC_B:
            break;
        case SDIO_PORT_XC_C:
            break;
        default:
            break;
    }

    return 0;
}

static void sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
	///do nothing here
	cpu_sdio_pwr_prepare(port);
}

static void sdio_pwr_on(unsigned port)
{
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            clrbits_le32(P_PREG_PAD_GPIO5_O,(1<<31)); //CARD_8
            clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
			/// @todo NOT FINISH
            break;
        case SDIO_PORT_C:    	
            break;
        case SDIO_PORT_XC_A:
            break;
        case SDIO_PORT_XC_B:
            break;
        case SDIO_PORT_XC_C:
            break;
        default:
            break;
    }
    return;
}
static void sdio_pwr_off(unsigned port)
{
    /// @todo NOT FINISH
    switch(port)
    {
        case SDIO_PORT_A:
            break;
        case SDIO_PORT_B:
            setbits_le32(P_PREG_PAD_GPIO5_O,(1<<31)); //CARD_8
            clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
            break;
        case SDIO_PORT_C:
            break;
        case SDIO_PORT_XC_A:
            break;
        case SDIO_PORT_XC_B:
            break;
        case SDIO_PORT_XC_C:
            break;
        default:
            break;
    }
    return;
}

// #define CONFIG_TSD      1
static void board_mmc_register(unsigned port)
{
    struct aml_card_sd_info *aml_priv=cpu_sdio_get(port);
    
    struct mmc *mmc = (struct mmc *)malloc(sizeof(struct mmc));
    if(aml_priv==NULL||mmc==NULL)
        return;
    memset(mmc,0,sizeof(*mmc));
    aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
    
// #ifdef CONFIG_TSD
    // // if(mmc->block_dev.dev > 0)//tsd
          // mmc->block_dev.if_type = IF_TYPE_SD;
// #else
    // // if(mmc->block_dev.dev > 0)//emmc
          // mmc->block_dev.if_type = IF_TYPE_MMC;
// #endif

	sdio_register(mmc, aml_priv);

#if 0    
    strncpy(mmc->name,aml_priv->name,31);
    mmc->priv = aml_priv;
	aml_priv->removed_flag = 1;
	aml_priv->inited_flag = 0;
	aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
	mmc->send_cmd = aml_sd_send_cmd;
	mmc->set_ios = aml_sd_cfg_swth;
	mmc->init = aml_sd_init;
	mmc->rca = 1;
	mmc->voltages = MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS;
	//mmc->host_caps = MMC_MODE_4BIT;
	mmc->bus_width = 1;
	mmc->clock = 300000;
	mmc->f_min = 200000;
	mmc->f_max = 50000000;
	mmc_register(mmc);
#endif	
}
int board_mmc_init(bd_t	*bis)
{
//board_mmc_register(SDIO_PORT_A);
	board_mmc_register(SDIO_PORT_B);
	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
#endif


#ifdef CONFIG_USB_DWC_OTG_HCD
#include <asm/arch/usb.h>
#include <asm/arch/gpio.h>
static void gpio_set_vbusa_power(char is_power_on)
{
    printf("%s\n", __FUNCTION__);
    if(is_power_on)
	    setbits_le32(P_PREG_PAD_GPIO1_O,(1<<17)); //GPIOZ_0
	else
	    clrbits_le32(P_PREG_PAD_GPIO1_O,(1<<17));
	clrbits_le32(P_PREG_PAD_GPIO1_EN_N,(1<<17));
}
static void gpio_set_vbusb_power(char is_power_on)
{
    printf("%s\n", __FUNCTION__);
    if(is_power_on)
	    setbits_le32(P_PREG_PAD_GPIO1_O,(1<<18)); //GPIOZ_1
	else
	    clrbits_le32(P_PREG_PAD_GPIO1_O,(1<<18));
	clrbits_le32(P_PREG_PAD_GPIO1_EN_N,(1<<18));
}
//note: try with some M3 pll but only following can work
//USB_PHY_CLOCK_SEL_M3_XTAL @ 1 (24MHz)
//USB_PHY_CLOCK_SEL_M3_XTAL_DIV2 @ 0 (12MHz)
//USB_PHY_CLOCK_SEL_M3_DDR_PLL @ 27(336MHz); @Rev2663 M3 SKT board DDR is 336MHz
//                                                            43 (528MHz); M3 SKT board DDR not stable for 528MHz
struct amlogic_usb_config g_usb_config_m6_skt={
	USB_PHY_CLK_SEL_XTAL,
	1, //PLL divider: (clock/12 -1)
	CONFIG_M8_USBPORT_BASE_B,
	USB_ID_MODE_SW_HOST,
	gpio_set_vbusb_power, //set_vbus_power
	NULL,
};
struct amlogic_usb_config g_usb_config_m6_skt_a={
	USB_PHY_CLK_SEL_XTAL,
	1, //PLL divider: (clock/12 -1)
	CONFIG_M8_USBPORT_BASE_A,
	USB_ID_MODE_HARDWARE,
	gpio_set_vbusa_power, //set_vbus_power
	NULL,
};
#endif /*CONFIG_USB_DWC_OTG_HCD*/
#ifdef CONFIG_IR_REMOTE_RECOVERY
void board_ir_init(void)
{
    writel(53<<16 | 50<<0,P_AO_MF_IR_DEC_LDR_ACTIVE);
    writel(31<<16 | 25<<0,P_AO_MF_IR_DEC_LDR_IDLE);
    writel(30<<16 | 26<<0,P_AO_MF_IR_DEC_LDR_REPEAT);
    writel(61<<16 | 55<<0,P_AO_MF_IR_DEC_BIT_0);
    writel(3<<28 |(0x5DC<<12)| 0x13,P_AO_MF_IR_DEC_REG0);
    writel((76<<20) | 69<<10,P_AO_MF_IR_DEC_STATUS);
    writel(0x9300,P_AO_MF_IR_DEC_REG1);
    writel(0x10b,P_AO_MF_IR_DEC_REG2);
    writel(91<<16 | 79<<0,P_AO_MF_IR_DEC_DURATN2);
    writel(111<<16 | 99<<0,P_AO_MF_IR_DEC_DURATN3);
    int val = readl(P_AO_RTI_PIN_MUX_REG);
    writel((val  | (1<<0)), P_AO_RTI_PIN_MUX_REG);
    printf("IR init done!\n");
}
int checkRecoveryKey(void)
{
    unsigned int keycode;
    if((P_AO_MF_IR_DEC_STATUS>>3)&0x1)
       keycode = readl(P_AO_MF_IR_DEC_FRAME);
    if(keycode==0x0008603d)
        return 1;
    else
        return 0;
}
//#define DEBUG_IR
#define msleep(a) udelay(a * 1000)
int do_irdetect(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int i;
#ifdef DEBUG_IR
    int j;
#endif
  board_ir_init();
#ifdef DEBUG_IR
  for(j=0;j<20;j++){
#endif
    for(i = 0; i < 1000000; i++)
        if(checkRecoveryKey()){
#ifdef DEBUG_IR
            printf("Detect Recovery Key ...\n");
#endif
            return 0;
            }
#ifdef DEBUG_IR
    msleep(50);
    printf("No key !!!\n");
  }
#endif
    return 1;
}
U_BOOT_CMD(
        irdetect, 1, 1, do_irdetect, 
        "Detect IR Key to start recovery system","[<string>]\n"
);
#endif /*CONFIG_IR_REMOTE_RECOVERY*/
#ifdef CONFIG_AML_I2C 
/*I2C module is board depend*/
static void board_i2c_set_pinmux(void){
	/*@AML9726-MX-MAINBOARD_V1.0.pdf*/
	/*@AL5631Q+3G_AUDIO_V1.pdf*/
    /*********************************************/
    /*                | I2C_Master_AO        |I2C_Slave            |       */
    /*********************************************/
    /*                | I2C_SCK                | I2C_SCK_SLAVE  |      */
    /* GPIOAO_4  | [AO_PIN_MUX: 6]     | [AO_PIN_MUX: 2]   |     */
    /*********************************************/
    /*                | I2C_SDA                 | I2C_SDA_SLAVE  |     */
    /* GPIOAO_5  | [AO_PIN_MUX: 5]     | [AO_PIN_MUX: 1]   |     */
    /*********************************************/	

	//disable all other pins which share with I2C_SDA_AO & I2C_SCK_AO
    clrbits_le32(P_AO_RTI_PIN_MUX_REG, ((1<<2)|(1<<24)|(1<<1)|(1<<23)));
    //enable I2C MASTER AO pins
	setbits_le32(P_AO_RTI_PIN_MUX_REG,
	(MESON_I2C_MASTER_AO_GPIOAO_4_BIT | MESON_I2C_MASTER_AO_GPIOAO_5_BIT));
	
    udelay(10000);
	
};

struct aml_i2c_platform g_aml_i2c_plat = {
    .wait_count         = 1000000,
    .wait_ack_interval  = 5,
    .wait_read_interval = 5,
    .wait_xfer_interval = 5,
    .master_no          = AML_I2C_MASTER_AO,
    .use_pio            = 0,
    .master_i2c_speed   = AML_I2C_SPPED_400K,
    .master_ao_pinmux = {
        .scl_reg    = MESON_I2C_MASTER_AO_GPIOAO_4_REG,
        .scl_bit    = MESON_I2C_MASTER_AO_GPIOAO_4_BIT,
        .sda_reg    = MESON_I2C_MASTER_AO_GPIOAO_5_REG,
        .sda_bit    = MESON_I2C_MASTER_AO_GPIOAO_5_BIT,
    }
};
#endif

#ifdef CONFIG_PLATFORM_HAS_PMU
static void board_pmu_init(void)
{
    struct aml_pmu_driver *driver = aml_pmu_get_driver();
    if (driver && driver->pmu_init) {
        driver->pmu_init();    
    }
}
#endif

inline void key_init(void)
{
}

inline int get_key(void)
{
    return 1;
}

static void board_i2c_init(void)
{		
	//set I2C pinmux with PCB board layout
	/*@AML9726-MX-MAINBOARD_V1.0.pdf*/
	/*@AL5631Q+3G_AUDIO_V1.pdf*/
	board_i2c_set_pinmux();

	//Amlogic I2C controller initialized
	//note: it must be call before any I2C operation
	aml_i2c_init();

	//must call aml_i2c_init(); before any I2C operation	
	/*M6 board*/
	//udelay(10000);	

	udelay(10000);
#ifdef CONFIG_PLATFORM_HAS_PMU
    board_pmu_init();
#endif
}

int board_init(void)
{
	gd->bd->bi_arch_number=MACH_TYPE_MESON6_SKT;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

    // LED
    clrbits_le32(P_AO_GPIO_O_EN_N, (1 << 15));     
    clrbits_le32(P_AO_GPIO_O_EN_N, (1 << 31));       

#ifdef CONFIG_AML_I2C  
	board_i2c_init();
#endif /*CONFIG_AML_I2C*/
#ifdef CONFIG_USB_DWC_OTG_HCD
	board_usb_init(&g_usb_config_m6_skt,BOARD_USB_MODE_HOST);
	board_usb_init(&g_usb_config_m6_skt_a,BOARD_USB_MODE_CHARGER);
    setbits_le32(P_PREG_PAD_GPIO1_O,(1<<18)); //GPIOZ_1
    clrbits_le32(P_PREG_PAD_GPIO1_EN_N,(1<<18));
#endif /*CONFIG_USB_DWC_OTG_HCD*/

	return 0;
}

static int do_msr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;

	/* need at least two arguments */
	if (argc > 2)
		goto usage;

	int nIndex = 0;
	int nCounter = 64;
	
	if( 2 == argc)
	{
		cmd = argv[1];
		char *endp;
		nIndex = simple_strtoul(argv[1], &endp, 10);
		if(nIndex < 0 || nIndex > 63)
			goto usage;
		nCounter = 1;
	}	
	
	extern unsigned long    clk_util_clk_msr(unsigned long clk_mux);

	//printf("\n");
	for(;((nIndex < 64) && nCounter);nCounter--,nIndex++)
		printf("MSR clock[%d] = %dMHz\n",nIndex,clk_util_clk_msr(nIndex));

	return 0;
	
usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	msr,	2, 	1,	do_msr,
	"Meson msr sub-system",
	" [0...63] - measure clock frequency\n"
	"          - no clock index will measure all clock"
);

static int do_checkhw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 1;

    ret = readl(P_PREG_PAD_GPIO2_I) >> 16 & 0x1F;
    switch(ret){
        case 2 :
            setenv("aml_dt", "m8_k01_2g");
            break;
        case 0 :
        default :
            setenv("aml_dt", "m8_k01_1g");
            break;
        }
    return 1;
}

U_BOOT_CMD(
        checkhw, 1, 1, do_checkhw, 
        "Get the hardware revsion","[<string>]\n"
);
