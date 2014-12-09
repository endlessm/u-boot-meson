/***********************************************
*****Storage config of board, for ACS use.*****
Header file: arch/arm/include/asm/arch-xx/storage.h
***********************************************/

#include <asm/arch/storage.h>

#ifdef CONFIG_ACS
//partition tables
struct partitions partition_table[MAX_PART_NUM]={
		{
			.name = "cache",
			.size = 192*SZ_1M,
			.mask_flags = STORE_DATA,
		},
		{
			.name = "env",
			.size = 8*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "logo",
			.size = 32*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "recovery",
			.size = 16*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "misc",
			.size = 32*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "boot",
			.size = 16*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "boot1",
			.size = 16*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "system",
			.size = 512*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "system1",
			.size = 512*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "persist",
			.size = 8*SZ_1M,
			.mask_flags = STORE_CODE,
		},
		{
			.name = "data",
			.size = NAND_PART_SIZE_FULL,
			.mask_flags = STORE_DATA,
		},
};

struct store_config  store_configs ={
		.store_device_flag = NAND_BOOT_FLAG,
		.nand_configs = {
			.enable_slc = 0,
			.order_ce = 0,
			.reserved = 0,
		},
		.mmc_configs = {
			.type = (PORT_A_CARD_TYPE | (PORT_B_CARD_TYPE << 4) | (PORT_C_CARD_TYPE << 8)),
			.port = 0,
			.reserved = 0,
		},
};


#endif
