/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"
#include "int10.h"
#include "int33.h"
#include "mpu.h"
#include "pic.h"
#include "timer.h"
#include "xms.h"

void bios_init()
{
	/* Start up our PIC and timer */
	pic_init();
	timer_init();
	emu_int33_init();
	mpu_init();
	xms_init();

	emu_get_memory16(BIOS_MEMORY_PAGE, 0, BIOS_VIDEO_CONTROLLER) = 0x3D4;

	/* Start the graphical part */
	if (getenv("TOC_TEST") == NULL) emu_int10_gfx(emu_get_memory8(BIOS_MEMORY_PAGE, 0, BIOS_VIDEO_MODE));
}

void bios_uninit(int exitCode)
{
	pic_uninit();
	timer_uninit();
	emu_int10_uninit(1);
	mpu_uninit();
	xms_uninit();

	/* Create a crash dump if there was an error */
	if (exitCode != 0) emu_make_crash_dump();

	exit(exitCode);
}
