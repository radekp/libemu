/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

void emu_int15()
{
	switch (emu_ax.h) {
		case 0x88: /* GET EXTENDED MEMORY SIZE */
		{          /* Return: AX -> extended size in paragraphs */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT15:88 ] GET EXTENDED MEMORY SIZE\n");
			emu_flags.cf = 0;
			emu_ax.x = 0;
		} return;

		default:
			fprintf(stderr, "[EMU] [ INT15:%02X ] Not Yet Implemented\n", emu_ax.h);
			emu_flags.cf = 1;
			return;
	}
}
