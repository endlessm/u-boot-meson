#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <malloc.h>

#if defined(CONFIG_CMD_NET)
#include <asm/arch/aml_eth_reg.h>
#include <asm/arch/aml_eth_pinmux.h>
#include <asm/arch/io.h>
#endif /*(CONFIG_CMD_NET)*/

#ifdef CONFIG_SARADC
#include <asm/saradc.h>
#endif /*CONFIG_SARADC*/

#if defined(CONFIG_AML_I2C)
#include <aml_i2c.h>
#include <asm/arch/io.h>
#endif /*CONFIG_AML_I2C*/


DECLARE_GLOBAL_DATA_PTR;


#if defined(CONFIG_CMD_NET)

/*************************************************
  * Amlogic Ethernet controller operation
  *
  * Note: The LAN chip LAN8720 need to be reset by GPIOY_15
  *
  *************************************************/
static void setup_net_chip(void)
{
#ifdef CONFIG_NET_RGMII
	/* setup ethernet clk */
	WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0x309);
	/* setup ethernet pinmux */
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, 0x4007ffe0);
	/* setup ethernet mode */
	WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, 0x211);
#elif defined(CONFIG_NET_RMII_CLK_EXTERNAL)
	/* setup ethernet clk */
	WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0x120);
	/* setup ethernet pinmux */
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, 0x8007ffe0);
	/* setup ethernet mode */
	WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, 0x241);
#else
	/* setup ethernet clk */
	WRITE_CBUS_REG(HHI_ETH_CLK_CNTL, 0x702);
	/* setup ethernet pinmux */
	SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, 0x4007ffe0);
	/* setup ethernet mode */
	WRITE_CBUS_REG(PREG_ETHERNET_ADDR0, 0x241);
#endif

	/* setup ethernet interrupt */
	SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN0_INTR_MASK, 1 << 8);
	SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN1_INTR_STAT, 1 << 8);

	/* hardware reset ethernet phy */
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO5_EN_N, 1 << 15);
	CLEAR_CBUS_REG_MASK(PREG_PAD_GPIO5_O, 1 << 15);
	udelay(2000);
	SET_CBUS_REG_MASK(PREG_PAD_GPIO5_O, 1 << 15);
}

int board_eth_init(bd_t *bis)
{
    setup_net_chip();

    udelay(1000);

#ifdef	CONFIG_USB_ETHER
    extern int usb_eth_initialize(bd_t *bi);
    usb_eth_initialize(bis);
#else
    extern int aml_eth_init(bd_t *bis);
    aml_eth_init(bis);
#endif

	return 0;
}
#endif /* (CONFIG_CMD_NET) */

#ifdef CONFIG_SARADC
/*following key value are test with board
  [M3_SKT_V1 20110622]
  ref doc:
  1. m3_skt_v1.pdf(2011.06.22)
  2. M3-Periphs-Registers.docx (Pg43-47)
*/
static struct adckey_info g_key_K1_info[] = {
    {"K1", 6, 60},
};
static struct adckey_info g_key_K2_info[] = {
    {"K2", 180, 60},
};
static struct adckey_info g_key_K3_info[] = {
    {"K3", 400, 60},
};
static struct adckey_info g_key_K4_info[] = {
    {"K4", 620, 60},
};
static struct adckey_info g_key_K5_info[] = {
    {"K5", 850, 60},
};

static struct adc_info g_adc_info[] = {
    {"Press Key K1", AML_ADC_CHAN_4, ADC_KEY,&g_key_K1_info},
    {"Press Key K2", AML_ADC_CHAN_4, ADC_KEY,&g_key_K2_info},
    {"Press Key K3", AML_ADC_CHAN_4, ADC_KEY,&g_key_K3_info},
    {"Press Key K4", AML_ADC_CHAN_4, ADC_KEY,&g_key_K4_info},
    {"Press Key K5", AML_ADC_CHAN_4, ADC_KEY,&g_key_K5_info},
    {"Press Key N/A",AML_ADC_CHAN_5, ADC_OTHER, NULL},
};

struct adc_device aml_adc_devices={
	.adc_device_info = g_adc_info,
	.dev_num = sizeof(g_adc_info)/sizeof(struct adc_info)
};

#endif

u32 get_board_rev(void)
{
    /*
    @todo implement this function
    */
	return 0x20;
}

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
static int  sdio_detect(unsigned port)
{
      int ret = 0;
      switch(port)
      {
            case SDIO_PORT_A:
                  break;
            case SDIO_PORT_B:
                  setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
                  ret=readl(P_PREG_PAD_GPIO5_I)&(1<<29);
                  printf( " %s return %d\n",__func__,ret);
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
      return ret;
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
}
static void sdio_pwr_off(unsigned port)
{
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
}
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
    sdio_register(mmc,aml_priv);

	/*
	* Normal procedure here, if you want init quickly for emmc,
	* do not set mmc->block_dev.if_type
	*/
	mmc->block_dev.if_type = IF_TYPE_SD;
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

#if CONFIG_AML_HDMI_TX
/*
 * Init hdmi related power configuration
 * Refer to your board SCH, power including HDMI5V, HDMI1.8V, AVDD18_HPLL, etc
 */
extern void hdmi_tx_power_init(void);
void hdmi_tx_power_init(void)
{
    // 
    printf("hdmi tx power init\n");
}
#endif

#ifdef CONFIG_AML_I2C 
/*I2C module is board depend*/
static void board_i2c_set_pinmux(void){
	/*@M6_SKT_V1.pdf*/
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


static void board_i2c_init(void)
{		
	//set I2C pinmux with PCB board layout
	/*@M6_SKT_V1.pdf*/
	/*@AL5631Q+3G_AUDIO_V1.pdf*/
	board_i2c_set_pinmux();

	//Amlogic I2C controller initialized
	//note: it must be call before any I2C operation
	aml_i2c_init();

	//must call aml_i2c_init(); before any I2C operation	
	/*M6 ref board*/
	//udelay(10000);	

	udelay(10000);		
}
#endif /*CONFIG_AML_I2C*/


#if CONFIG_JERRY_NAND_TEST //temp test
#include <amlogic/nand/platform.h>
#include <asm/arch/nand.h>
#include <linux/mtd/partitions.h>
static void claim_bus(uint32_t get)
{
	if(get==NAND_BUS_RELEASE)
	{
		NAND_IO_DISABLE(0);
	}else{
		NAND_IO_ENABLE(0);
	}
}
static struct aml_nand_platform nand_plat={
/*
		uint32_t        reg_base;
		    uint32_t        delay;
		    uint32_t        rbmod;
		    uint32_t        t_rea;
		    uint32_t        t_rhoh;
		    uint32_t        ce_num;
		    uint32_t        clk_src;
		    claim_bus_t     claim_bus;
*/
		.ce_num=4,
		.rbmod=1,
};
void    board_nand_init(void)
{
	nanddebug("NAND is inited\n");
	nand_probe(&nand_plat);
//	cntl_init(&nand_plat);
//	amlnand_probe();
}
#elif CONFIG_NAND_AML_M3 //temp test
//#include <amlogic/nand/platform.h>
#include <asm/arch/nand.h>
#include <linux/mtd/partitions.h>


static struct aml_nand_platform aml_nand_mid_platform[] = {
#if defined CONFIG_SPI_NAND_COMPATIBLE || defined CONFIG_SPI_NAND_EMMC_COMPATIBLE
    {
        .name = NAND_BOOT_NAME,
        .chip_enable_pad = AML_NAND_CE0,
        .ready_busy_pad = AML_NAND_CE0,
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH60_1K_MODE),
            },
        },
        .rbpin_mode=1,
        .short_pgsz=384,
        .ran_mode=0,
        .T_REA = 20,
        .T_RHOH = 15,
    },
#endif
    {
        .name = NAND_NORMAL_NAME,
        .chip_enable_pad = (AML_NAND_CE0) | (AML_NAND_CE1 << 4),// | (AML_NAND_CE2 << 8) | (AML_NAND_CE3 << 12)),
        .ready_busy_pad = (AML_NAND_CE0) | (AML_NAND_CE1 << 4),// | (AML_NAND_CE1 << 8) | (AML_NAND_CE1 << 12)),
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 2,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH60_1K_MODE | NAND_TWO_PLANE_MODE),
            },
        },
        .rbpin_mode = 1,
        .short_pgsz = 0,
        .ran_mode = 0,
        .T_REA = 20,
        .T_RHOH = 15,
    }
    
};

struct aml_nand_device aml_nand_mid_device = {
    .aml_nand_platform = aml_nand_mid_platform,
    .dev_num = ARRAY_SIZE(aml_nand_mid_platform),
};
#endif

#ifdef CONFIG_USB_DWC_OTG_HCD
#include <asm/arch/usb.h>
#include <asm/arch/gpio.h>
static void gpio_set_vbus_power(char is_power_on)
{
	if(is_power_on)
	{
		//GPIOD_7 -- USB_PWR_CTL
		set_gpio_mode(GPIOD_bank_bit0_9(7), GPIOD_bit_bit0_9(7), GPIO_OUTPUT_MODE);
		set_gpio_val(GPIOD_bank_bit0_9(7), GPIOD_bit_bit0_9(7), 1);
		udelay(100000);
	}
	else
	{
		set_gpio_mode(GPIOD_bank_bit0_9(7), GPIOD_bit_bit0_9(7), GPIO_OUTPUT_MODE);
		set_gpio_val(GPIOD_bank_bit0_9(7), GPIOD_bit_bit0_9(7), 0);
	}
}

//note: try with some M3 pll but only following can work
//USB_PHY_CLOCK_SEL_M3_XTAL @ 1 (24MHz)
//USB_PHY_CLOCK_SEL_M3_XTAL_DIV2 @ 0 (12MHz)
//USB_PHY_CLOCK_SEL_M3_DDR_PLL @ 27(336MHz); @Rev2663 M3 SKT board DDR is 336MHz
//                                                            43 (528MHz); M3 SKT board DDR not stable for 528MHz
struct amlogic_usb_config g_usb_config_m6_skt={
	USB_PHY_CLK_SEL_XTAL,
	1, //PLL divider: (clock/12 -1)
	CONFIG_M6_USBPORT_BASE,
	USB_ID_MODE_SW_HOST,
	gpio_set_vbus_power, //set_vbus_power
	NULL,
};
#endif /*CONFIG_USB_DWC_OTG_HCD*/

int board_init(void)
{
	gd->bd->bi_arch_number=MACH_TYPE_MESON6_REF;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

	setbits_le32(P_AO_GPIO_O_EN_N,1<<31);//TEST_N, VCCK enable
        //GPIOAO_4, SELF_INIT_RST# is high
        clrbits_le32(P_AO_GPIO_O_EN_N,1<<4);
        setbits_le32(P_AO_GPIO_O_EN_N,1<<20);
        //GPIOAO_5, STBY# is high
        clrbits_le32(P_AO_GPIO_O_EN_N,1<<5);
        setbits_le32(P_AO_GPIO_O_EN_N,1<<21);

	return 0;
}

#ifdef	BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_AML_I2C  
	board_i2c_init();
#endif /*CONFIG_AML_I2C*/

#ifdef CONFIG_USB_DWC_OTG_HCD
	board_usb_init(&g_usb_config_m6_skt,BOARD_USB_MODE_HOST);
#endif /*CONFIG_USB_DWC_OTG_HCD*/

	return 0;
}
#endif


//POWER key
inline void key_init(void)
{
	clrbits_le32(P_RTC_ADDR0, (1<<11));
	clrbits_le32(P_RTC_ADDR1, (1<<3));
}

inline int get_key(void)
{
	return (((readl(P_RTC_ADDR1) >> 2) & 1) ? 0 : 1);
}
#ifdef CONFIG_SWITCH_BOOT_MODE
int switch_boot_mode(void)
{
	printf("switch_boot_mode\n");
	unsigned int suspend_status_current2 = readl(P_AO_RTI_STATUS_REG2);
	printf("suspend_status_current2=%x\n",suspend_status_current2);
	if((suspend_status_current2 == 0))
	{
		run_command ("suspend", 0);
	}
	return 0;
}
#endif

#ifdef CONFIG_AML_TINY_USBTOOL
	int usb_get_update_result(void)
	{
		unsigned long upgrade_step;
		upgrade_step = simple_strtoul (getenv ("upgrade_step"), NULL, 16);
		printf("upgrade_step = %d\n", upgrade_step);
		if(upgrade_step == 1)
		{
			run_command("defenv", 1);
			run_command("setenv upgrade_step 2", 1);
			run_command("saveenv", 1);
			return 0;
		}
		else
		{
			return -1;
		}
	}
#endif

#ifdef CONFIG_IR_RECOVERY_DETECT
#define POWER_KEYCODE                   0x2180f4
#define REMOTE_DUOKAN_STATUS_WAIT       0
#define REMOTE_DUOKAN_STATUS_INTERVAL   1
#define REMOTE_DUOKAN_STATUS_DATA       2
#define REMOTE_DUOKAN_STATUS_FACTORY    3
#define REMOTE_DUOKAN_STATUS_LEADER     4
#define REMOTE_DUOKAN_STATUS_NUM        4
unsigned int time_window[10];
unsigned int state;
unsigned int keycode;
unsigned int frame_bit_num = 20;
unsigned int bit_num = 2;
int bit;
void init_custom_trigger(void)
{
    int val = readl(P_AO_RTI_PIN_MUX_REG);
    writel((val  | (1<<0)), P_AO_RTI_PIN_MUX_REG);
    writel(0x8148, P_AO_IR_DEC_REG1);//use 1us
    writel(0xfa004, P_AO_IR_DEC_REG0);//5us
    time_window[0] = 0x131;//1596/5  leader 13f
    time_window[1] = 0x149;//1596/5  leader
    time_window[2] = 0xdb;//1176/5  00
    time_window[3] = 0xf9;//1420/5  01
    time_window[4] = 0x10f;//1764/5  10
    time_window[5] = 0x12b;//2088/5   11
    time_window[6] = 0x14f;//1176/5  00
    time_window[7] = 0x163;//1420/5  01
    time_window[8] = 0x18f;//1764/5  10
    time_window[9] = 0x201;//2088/5   11
    state = REMOTE_DUOKAN_STATUS_WAIT;
    // enable interrupt
    writel(readl(P_AO_IRQ_MASK_FIQ_SEL)|(1<<4), P_AO_IRQ_MASK_FIQ_SEL);
}
int checkRecoveryKey()
{
    unsigned int pluse;
    unsigned int i;
    // wait interrupt
    if(!(readl(P_AO_IRQ_STAT_CLR)&(1<<4))) 
        return 0;
    pluse = (readl(P_AO_IR_DEC_REG1)>>16)&0x1fff;
    if(pluse > 0x210 ) // null
        state = REMOTE_DUOKAN_STATUS_WAIT;
    switch (state) {
        case REMOTE_DUOKAN_STATUS_LEADER:
            if((pluse < time_window[1])&&(pluse > time_window[0]))
                state = REMOTE_DUOKAN_STATUS_DATA;
            return 0;
        case REMOTE_DUOKAN_STATUS_DATA:
            for(i=0; i<REMOTE_DUOKAN_STATUS_NUM; i++)
                if((pluse <= time_window[2*i+3])&&(pluse > time_window[2*i+2]))
                    keycode |= i<<bit;
                    bit -= bit_num;
                    break;
            if(bit <0) bit = 0;
            if(i == REMOTE_DUOKAN_STATUS_NUM) bit = 0;
            if(bit == 0)
                state = REMOTE_DUOKAN_STATUS_WAIT;
            else
                return 0;
            break;
        case REMOTE_DUOKAN_STATUS_WAIT:
        default:
            keycode = 0;
            bit = frame_bit_num;
            state = REMOTE_DUOKAN_STATUS_LEADER;
            return 0;
        }
    //printf("keycode = %x-- bit = %d\n",keycode,bit);
    if(keycode==POWER_KEYCODE)
        return 1;
    else 
        return 0;
}
void clear_custom_trigger(void)
{
    int val = readl(P_AO_RTI_PIN_MUX_REG);
    writel((val & ~(1<<0)), P_AO_RTI_PIN_MUX_REG);
    writel(readl(P_AO_IRQ_MASK_FIQ_SEL)|(1<<20), P_AO_IRQ_MASK_FIQ_SEL);
    writel(0, P_AO_IRQ_MASK_FIQ_SEL);
}
#define msleep(a) udelay(a * 1000)
//#define DEBUG_IR
int do_irdetect(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int i;
#ifdef DEBUG_IR
    int j;
#endif
    init_custom_trigger();
#ifdef DEBUG_IR
  for(j=0;j<20;j++){
#endif
    for(i = 0; i < 1000000; i++)
        if(checkRecoveryKey()){
#ifdef DEBUG_IR
            printf("Detect Recovery Key ...\n");
#endif
            clear_custom_trigger();
            return 0;
            }
#ifdef DEBUG_IR
    msleep(50);
    printf("No key !!!\n");
  }
#endif
    clear_custom_trigger(); 
    return 1;
}

U_BOOT_CMD(irdetect, 1, 1, do_irdetect, "Detect IR Key to start recovery system","[<string>]\n");
#endif /*CONFIG_IR_RECOVERY_DETECT*/
