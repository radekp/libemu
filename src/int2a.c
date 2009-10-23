#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

void emu_int2a()
{
	switch (emu_ah) {
		case 0x00: /* GET INSTALLED NETWORK STATE */
		{            /* Return: AH -> 0x00 (not installed), anything else (installed) */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT2A:00 ] GET NETWORK INSTALLED STATE '%X'\n", emu_ah);
			emu_ah = 0x00; /* Not installed */
		} return;

		default:
			fprintf(stderr, "[EMU] [ INT2A:%02X ] Not Yet Implemented\n", emu_ah);
			bios_uninit(1);
	}
}
