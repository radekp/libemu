/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

void emu_int2f()
{
	switch (emu_al) {
		case 0x00: /* GET INSTALLED STATE - AH -> application number */
		{          /* Return: AL -> 0x00 (not installed, ok to install), 0x01, (not installed, not ok to install), 0xFF (installed) */
			switch (emu_ah) {
				case 0x43: /* Himem XMS Driver */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT2F:00 ] GET INSTALLED STATE 'Himen XMS driver'\n");
					emu_al = 0x80;
					return;

				default:
					fprintf(stderr, "[EMU] [ INT2F:00 ] Application '%X' Not Yet Implemented\n", emu_ah);
					bios_uninit(1);
			}
		} return;

		case 0x10: /* GET DRIVER CONTROL FUNCTION - AH -> application number */
		{          /* Return: ES:BX -> pointer to control function */
			switch (emu_ah) {
				case 0x43: /* Himem XMS Driver */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT2F:10 ] GET DRIVER CONTROL FUNCTION 'Himen XMS driver'\n");
					emu_es = 0x70;
					emu_bx = 8 * 250;

					/* First, jump to the next blob, allowing jump-linking.
					 *  This is required for all XMS implementations */
					emu_get_memory8(emu_es, emu_bx, 0) = 0xEB;
					emu_get_memory8(emu_es, emu_bx, 1) = 0x06;
					emu_get_memory8(emu_es, emu_bx, 2) = 0x90;
					emu_get_memory8(emu_es, emu_bx, 3) = 0x90;
					emu_get_memory8(emu_es, emu_bx, 4) = 0x90;
					emu_get_memory8(emu_es, emu_bx, 5) = 0x90;
					emu_get_memory8(emu_es, emu_bx, 6) = 0x90;
					emu_get_memory8(emu_es, emu_bx, 7) = 0x90;

					/* Make this syscall do a retf, not a reti */
					emu_get_memory8(emu_es, emu_bx, 12) = 0xCB;

					emu_ax = 1;
					return;

				default:
					fprintf(stderr, "[EMU] [ INT2F:10 ] Application '%X' Not Yet Implemented\n", emu_ah);
					bios_uninit(1);
			}
		} return;

		default:
			fprintf(stderr, "[EMU] [ INT2F:%04X ] Not Yet Implemented\n", emu_ax);
			bios_uninit(1);
	}
}
