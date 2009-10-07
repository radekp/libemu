/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

void emu_int13()
{
	switch (emu_ah) {
		case 0x00: /* RESET DISK - DL -> drive */
		{          /* Return: AH -> status */
			/* Sure thing, I will do that right away ... NOT! */
			emu_flags.cf = 0;
			emu_ah = 0;
		} return;

		case 0x08: /* GET DRIVE PARAMETERS - DL -> drive number */
		{          /* Return: AH -> status, BL -> CMOS type, CH -> cylinders, CL -> sectors per track, DH -> sides, DL -> drives, ES:DI -> DBT */
			/* TODO -- Implement this */
		} return;

		/* Used by 'checkit', No idea. Ignore. */
		case 0x80: return;

		default:
			fprintf(stderr, "[EMU] [ INT13:%02X ] Not Yet Implemented\n", emu_ah);
			bios_uninit(1);
	}
}
