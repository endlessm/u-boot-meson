#ifndef MESON_POC_H
#define MESON_POC_H

#define R_BOOT_DEVICE_FLAG  READ_CBUS_REG(ASSIST_POR_CONFIG)
#define POR_BOOT_VALUE 	((((R_BOOT_DEVICE_FLAG>>9)&1)<<2)|((R_BOOT_DEVICE_FLAG>>6)&3))

#ifndef CONFIG_FORCE_EMMC_BOOT

#define POR_NAND_BOOT() ((POR_BOOT_VALUE == 7) || (POR_BOOT_VALUE == 6))
#define POR_SPI_BOOT()  ((POR_BOOT_VALUE == 5) || (POR_BOOT_VALUE == 4))

#define POR_EMMC_BOOT() ((POR_BOOT_VALUE == 3) || (POR_BOOT_VALUE == 1))
#define POR_CARD_BOOT() (POR_BOOT_VALUE == 0)

#else

#define POR_NAND_BOOT()			(0)
#define POR_SPI_BOOT()			(0)
#define POR_EMMC_BOOT()			(1)
#define POR_CARD_BOOT()			(0)

#endif

#endif
