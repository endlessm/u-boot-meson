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

DEV_AML_BOARD=m8_k06_NabiJR_v1

echo "............patching lcd ........................"
#patch for clock_spread_spectrum 
funpatch arch/arm/cpu/aml_meson/m8/lcdoutc.c customer/board/m8_k06_NabiJR_512M/patch/lcdoutc.c.patch
