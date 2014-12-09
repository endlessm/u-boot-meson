#ifdef CONFIG_IR_REMOTE_WAKEUP

#define IR_CONTROL_HOLD_LAST_KEY   (1<<6)

unsigned backup_AO_RTI_PIN_MUX_REG;
unsigned backup_AO_IR_DEC_REG0;
unsigned backup_AO_IR_DEC_REG1;
unsigned backup_AO_IR_DEC_LDR_ACTIVE;
unsigned backup_AO_IR_DEC_LDR_IDLE;
unsigned backup_AO_IR_DEC_BIT_0;

/*****************************************************************
**
** func : ir_remote_init
**       in this function will do pin configuration and and initialize for
**       IR Remote hardware decoder mode at 32kHZ on ARC.
**
********************************************************************/
void backup_remote_register(void)
{
    backup_AO_RTI_PIN_MUX_REG = readl(P_AO_RTI_PIN_MUX_REG);
    backup_AO_IR_DEC_REG0 = readl(P_AO_IR_DEC_REG0);
    backup_AO_IR_DEC_REG1 = readl(P_AO_IR_DEC_REG1);
    backup_AO_IR_DEC_LDR_ACTIVE = readl(P_AO_IR_DEC_LDR_ACTIVE);
    backup_AO_IR_DEC_LDR_IDLE = readl(P_AO_IR_DEC_LDR_IDLE);
    backup_AO_IR_DEC_BIT_0 = readl(P_AO_IR_DEC_BIT_0);
}

void resume_remote_register(void)
{
	writel(backup_AO_RTI_PIN_MUX_REG,P_AO_RTI_PIN_MUX_REG);
	writel(backup_AO_IR_DEC_REG0,P_AO_IR_DEC_REG0);
	writel(backup_AO_IR_DEC_REG1,P_AO_IR_DEC_REG1);
	writel(backup_AO_IR_DEC_LDR_ACTIVE,P_AO_IR_DEC_LDR_ACTIVE);
	writel(backup_AO_IR_DEC_LDR_IDLE,P_AO_IR_DEC_LDR_IDLE);
	writel(backup_AO_IR_DEC_BIT_0,P_AO_IR_DEC_BIT_0);

	readl(P_AO_IR_DEC_FRAME);//abandon last key
}

static int ir_remote_init_32k_mode(void)
{
    unsigned int control_value,status,data_value;
    int i;

    int val = readl(P_AO_RTI_PIN_MUX_REG);
		writel((val  | (1<<0)), P_AO_RTI_PIN_MUX_REG);
		
		control_value = 0x600ffe00;
    writel( control_value,P_AO_IR_DEC_REG1 );
    
    control_value = (0xa3f << 12) | 0;

    writel(control_value,P_AO_IR_DEC_REG0);
	
		// no filter
     writel((readl(P_AO_IR_DEC_REG0)& ~(0x7 << 28)) | (0 << 28),P_AO_IR_DEC_REG0);
    
    // SCALE LEADER ACTIVE
    writel((readl(P_AO_IR_DEC_LDR_ACTIVE)& ~(0x3FF << 16)) | ((unsigned)(294*1.1) << 16),P_AO_IR_DEC_LDR_ACTIVE);
    writel((readl(P_AO_IR_DEC_LDR_ACTIVE)& ~(0x3FF << 0)) | ((unsigned)(294*0.9) << 0),P_AO_IR_DEC_LDR_ACTIVE);

    // SCALE LEADER IDLE
    writel((readl(P_AO_IR_DEC_LDR_IDLE)& ~(0x3FF << 16)) | ((unsigned)(147*1.1) << 16),P_AO_IR_DEC_LDR_IDLE);
    writel((readl(P_AO_IR_DEC_LDR_IDLE)& ~(0x3FF << 0)) | ((unsigned)(147*0.9) << 16),P_AO_IR_DEC_LDR_IDLE);


    // SCALE BIT 0 (1.11mS)
    writel((readl(P_AO_IR_DEC_BIT_0)& ~(0x3FF << 16)) | ((unsigned)(36*1.1) << 16),P_AO_IR_DEC_BIT_0);
    writel((readl(P_AO_IR_DEC_BIT_0)& ~(0x3FF << 0))  | ((unsigned)(36*0.9) << 0),P_AO_IR_DEC_BIT_0);


    status = readl(P_AO_IR_DEC_STATUS);
    data_value = readl(P_AO_IR_DEC_FRAME);

    //step 2 : request nec_remote irq  & enable it
    return 0;
}

void init_custom_trigger(void)
{
	ir_remote_init_32k_mode();
}
#endif
