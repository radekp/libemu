/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

void emu_int16()
{
	switch (emu_ah) {
		case 0x00: /* WAIT AND READ KEYPRESS */
		{          /* Return: AH -> scan code, AL ->ASCII char */
			extern int emu_int9_key_getascii();
			emu_ax = emu_int9_key_getascii();
		} return;

		case 0x01: /* GET KEYBOARD STATUS */
		{          /* Return: ZF -> 0 if key pressed, AX -> 0 if no scancode, AH -> scan code, AL -> ASCII char */
			extern int emu_int9_key_getascii();
			extern int emu_int9_key_iswaiting();

			if (emu_int9_key_iswaiting()) {
				emu_flags.zf = 0;
				emu_ax = emu_int9_key_getascii();
			} else {
				emu_flags.zf = 1;
				emu_ax = 0x0000;
			}
		} return;

		default:
			fprintf(stderr, "[EMU] [ INT16:%02X ] Not Yet Implemented\n", emu_ah);
			bios_uninit(1);
	}
}
