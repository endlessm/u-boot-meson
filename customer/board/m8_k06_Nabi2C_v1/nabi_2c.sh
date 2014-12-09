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

DEV_AML_BOARD=m8_k06_Nabi2C_v1
funpatch   arch/arm/cpu/aml_meson/common/firmware/power.c	customer/board/$DEV_AML_BOARD/led_patch/power.c.patch
funpatch   arch/arm/cpu/aml_meson/m8/firmware/arc_power/pwr_op.c  customer/board/$DEV_AML_BOARD/led_patch/power.c.patch 
funpatch   common/cmd_aml.c  customer/board/$DEV_AML_BOARD/led_patch/power.c.patch
funpatch   drivers/power/rn5t618.c customer/board/$DEV_AML_BOARD/led_patch/power.c.patch

echo "Finished patch files copy!!!"
