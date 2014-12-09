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

DEV_AML_BOARD=m6_skyworth_m1701

#nand patch
echo "............patching lcd ........................"
#cp -v ./cmd_usid   ../../../common/cmd_usid.c
#cp -v ./burn_func  ../../../drivers/usb/gadget/aml_tiny_usbtool/burn_func.c
funpatch	arch/arm/cpu/aml_meson/m6/firmware/arc_power/config.h  customer/board/$DEV_AML_BOARD/arc_power.config_patch
funpatch	drivers/video/lcd/aml_lcd.c   customer/board/$DEV_AML_BOARD/aml_lcd.c_patch

