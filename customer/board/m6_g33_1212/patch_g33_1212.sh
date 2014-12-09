#! /bin/bash


funpatch()
{     
    if [ -n "$1"  -a -n "$2" ];then
		dirpath=$(dirname $1)
		cd $dirpath
		filename=$(basename $1)
		git checkout $filename
		cd -
		patch $1 < $2		
	else
		echo "error: please fill function argument.............. "
		exit
	fi

}


#ddr patch
echo "............patching ddr ........................"
#funpatch	arch/arm/cpu/aml_meson/m6/firmware/arc_power/mmc.c customer/board/m6_g33_1212/patch/ddr_patch/mmc.diff
#funpatch	arch/arm/cpu/aml_meson/m6/firmware/ddr_init_pctl.c customer/board/m6_g33_1212/patch/ddr_patch/ddr_init_pctl.diff

echo "............patching AM1212 ...................."
funpatch	arch/arm/cpu/aml_meson/m6/firmware/arc_power/config.h customer/board/m6_g33_1212/patch/AM1212_patch/config.h.diff
#funpatch	arch/arm/cpu/aml_meson/m6/firmware/arc_power/test.c customer/board/m6_g33_1212/patch/AM1212_patch/test.c.diff
#funpatch	arch/arm/cpu/aml_meson/m6/firmware/arc_power/i2c.c customer/board/m6_g33_1212/patch/AM1212_patch/i2c.c.diff
#funpatch	common/main.c customer/board/m6_g33_1212/patch/AM1212_patch/main.c.diff
#funpatch	drivers/power/Makefile customer/board/m6_g33_1212/patch/AM1212_patch/Makefile.diff

#cp customer/board/m6_g33_1212/patch/AM1212_patch/aml_pmu.c drivers/power/aml_pmu.c
#cp customer/board/m6_g33_1212/patch/AM1212_patch/aml_pmu.h include/amlogic/aml_pmu.h


