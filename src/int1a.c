/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

void emu_int1a()
{
	switch (emu_ah) {
		case 0: /* READ SYSTEM CLOCK */
		{       /* Return: AL -> 0, CX:DX -> system clock */
			emu_al = emu_get_memory8 (BIOS_MEMORY_PAGE, 0, BIOS_COUNTER_OVERFLOW);
			emu_cx = emu_get_memory32(BIOS_MEMORY_PAGE, 0, BIOS_COUNTER) >> 16;
			emu_dx = emu_get_memory32(BIOS_MEMORY_PAGE, 0, BIOS_COUNTER) & 0xFFFF;
		} return;

		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
			fprintf(stderr, "[EMU] [ INT1A:%02X ] Tandy soundsystem not supported\n", emu_ah);
			break;

		default:
			fprintf(stderr, "[EMU] [ INT1A:%02X ] Not Yet Implemented\n", emu_ah);
			bios_uninit(1);
	}
}
