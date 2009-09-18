#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

void emu_int2f()
{
	switch (emu_ax.x) {
		case 0x4300: /* GET INSTALLED STATE - AH -> application number */
		{            /* Return: AL -> 0x00 (not installed, ok to install), 0x01, (not installed, not ok to install), 0xFF (installed) */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT2F:00 ] GET INSTALLED STATE '%X'\n", emu_ax.h);
			emu_ax.l = 0x01; // Not Installed and do not install
		} return;

		default:
			fprintf(stderr, "[EMU] [ INT2F:%04X ] Not Yet Implemented\n", emu_ax.x);
			bios_uninit(1);
	}
}
