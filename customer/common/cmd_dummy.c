/*
 * Command for user defined command.
 *
 * Copyright (C) 2012 Amlogic Corporation
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/io.h>

static int do_dummy_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	puts("Customer defined command,please implement it!\n");

 	return 0;
}

U_BOOT_CMD(
	dummy,	1,	1,	do_dummy_cmd,
	"Customer defined command!",
	""
);
