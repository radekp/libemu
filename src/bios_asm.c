/* $Id$ */

#include <stdio.h>
#if defined(_MSC_VER)
#	include <windows.h>
#	define usleep Sleep
#else
#	include <unistd.h>
#endif /* _MSC_VER */
#include "types.h"
#include "libemu.h"
#include "bios.h"
#include "pic.h"
#include "timer.h"
#include "int10.h"
#include "int13.h"
#include "int15.h"
#include "int16.h"
#include "int1a.h"
#include "int21.h"
#include "int2f.h"
#include "int33.h"

uint8 emu_io_read_005()
{
	/* TODO -- DMA is used for Memory access. For now we patch it a bit
	 *  so it works with Dune2, but this of course is no real solution */
	static uint8 last = 0x00;
	if (last == 0x00) last = 0x08;
	else last = 0x00;

	return last;
}

/* IN */

void emu_inb(uint8 *dest, uint16 port)
{
	if (emu_debug_int) fprintf(stderr, "[EMU] [ INB:%02X ]\n", port);
	switch (port) {
		case 0x005: *dest = emu_io_read_005(); return; // TODO
		case 0x006: *dest = emu_io_read_005(); return; // TODO
		case 0x021: *dest = 0xFF; return; // TODO
		case 0x040: *dest = emu_io_read_040(); return;
		case 0x042: *dest = emu_io_read_042(); return;
		case 0x060: *dest = emu_io_read_060(); return;
		case 0x061: *dest = 0xFF; return; // TODO
		case 0x071: *dest = 0xFF; return; // TODO -- No clue
		case 0x201: *dest = 0xFF; return; // TODO -- No clue
		case 0x228: *dest = 0xFF; return; // TODO -- No clue
		case 0x22A: *dest = 0xFF; return; // TODO -- No clue
		case 0x22B: *dest = 0xFF; return; // TODO -- No clue
		case 0x22E: *dest = 0xFF; return; // TODO -- No clue
		case 0x23B: *dest = 0xFF; return; // TODO -- No clue
		case 0x24B: *dest = 0xFF; return; // TODO -- No clue
		case 0x25B: *dest = 0xFF; return; // TODO -- No clue
		case 0x26B: *dest = 0xFF; return; // TODO -- No clue
		case 0x278: *dest = 0xFF; return; // TODO -- Parallel port
		case 0x27B: *dest = 0xFF; return; // TODO -- No clue
		case 0x288: *dest = 0xFF; return; // TODO -- No clue
		case 0x2ED: *dest = 0xFF; return; // TODO -- No clue
		case 0x2FD: *dest = 0xFF; return; // TODO -- No clue
		case 0x318: *dest = 0xFF; return; // TODO -- No clue
		case 0x378: *dest = 0xFF; return; // TODO -- Parellel port
		case 0x388: *dest = 0xFF; return; // TODO -- 8273 stuff
		case 0x3B5: *dest = 0xFF; return; // TODO -- Monochrome Display stuff
		case 0x3BC: *dest = 0xFF; return; // TODO -- Parallel port
		case 0x3C7: *dest = emu_io_read_3C7(); return;
		case 0x3C9: *dest = emu_io_read_3C9(); return;
		case 0x3D5: *dest = 0xFF; return; // TODO -- Colour Display stuff
		case 0x3D9: *dest = emu_io_read_3D9(); return;
		case 0x3DA: *dest = emu_io_read_3DA(); return;
		case 0x3ED: *dest = 0xFF; return; // TODO -- No clue
		case 0x3FD: *dest = 0xFF; return; // TODO -- No clue
		default:
			fprintf(stderr, "[EMU] [ INB:%02X ] Not Yet Implemented\n", port);
			bios_uninit(1);
			return;
	}
}
void emu_inw(uint16 *dest, uint16 port) {
	if (emu_debug_int) fprintf(stderr, "[EMU] [ INW:%02X ]\n", port);
	switch (port) {
		case 0x60: *dest = emu_io_read_060(); return;
		default:
			fprintf(stderr, "[EMU] [ INW:%02X ] Not Yet Implemented\n", port);
			bios_uninit(1);
			return;
	}
}

/* INS */

void emu_insb()
{
	emu_inb(&emu_get_memory8(emu_es, emu_di, 0), emu_dx.x);
	if (emu_flags.df) emu_di -= 1; else emu_di += 1;
}
void emu_insw()
{
	emu_inw(&emu_get_memory16(emu_es, emu_di, 0), emu_dx.x);
	if (emu_flags.df) emu_di -= 2; else emu_di += 2;
}

/* OUT */

void emu_outb(uint16 port, uint8 value) {
	if (emu_debug_int) fprintf(stderr, "[EMU] [ OUTB:%02X ] VALUE: 0x%02X\n", port, value);
	switch (port) {
		case 0x000: return; // TODO
		case 0x001: return; // TODO
		case 0x020: return; // TODO
		case 0x021: return; // TODO
		case 0x040: emu_io_write_040(value); return;
		case 0x042: emu_io_write_042(value); return;
		case 0x043: emu_io_write_043(value); return;
		case 0x061: return; // TODO -- Sound?
		case 0x070: return; // TODO -- No clue
		case 0x071: return; // TODO -- No clue
		case 0x201: return; // TODO -- No clue
		case 0x226: return; // TODO -- No clue
		case 0x227: return; // TODO -- No clue
		case 0x228: return; // TODO -- No clue
		case 0x229: return; // TODO -- No clue
		case 0x22A: return; // TODO -- No clue
		case 0x237: return; // TODO -- No clue
		case 0x247: return; // TODO -- No clue
		case 0x257: return; // TODO -- No clue
		case 0x267: return; // TODO -- No clue
		case 0x277: return; // TODO -- No clue
		case 0x278: return; // TODO -- Parallel port
		case 0x288: return; // TODO -- No clue
		case 0x289: return; // TODO -- No clue
		case 0x318: return; // TODO -- No clue
		case 0x319: return; // TODO -- No clue
		case 0x378: return; // TODO -- Parallel port
		case 0x388: return; // TODO -- 8273 stuff
		case 0x389: return; // TODO -- 8273 stuff
		case 0x3B4: return; // TODO -- Monochrome Display stuff
		case 0x3B5: return; // TODO -- Monochrome Display stuff
		case 0x3BC: return; // TODO -- Parallel port
		case 0x3BF: return; // TODO -- Printer?
		case 0x3C0: return; // TODO -- VGA attribute stuff
		case 0x3C2: return; // TODO -- Display stuff
		case 0x3C4: return; // TODO -- Display stuff
		case 0x3C5: return; // TODO -- Display stuff
		case 0x3C7: emu_io_write_3C7(value); return;
		case 0x3C8: emu_io_write_3C8(value); return;
		case 0x3C9: emu_io_write_3C9(value); return;
		case 0x3CE: return; // TODO -- VGA Register stuff
		case 0x3CF: return; // TODO -- VGA Register stuff
		case 0x3D4: return; // TODO -- Colour Display stuff
		case 0x3D5: return; // TODO -- Colour Display stuff
		case 0x3D8: return; // TODO -- Mode Control
		case 0x3D9: emu_io_write_3D9(value); return;
		case 0x3F2: return; // TODO -- Floppy Disk stuff
		default:
			fprintf(stderr, "[EMU] [ OUTB:%02X ] Not Yet Implemented\n", port);
			bios_uninit(1);
			return;
	}
}
void emu_outw(uint16 port, uint16 value) {
	if (emu_debug_int) fprintf(stderr, "[EMU] [ OUTW:%02X ] VALUE: 0x%04X\n", port, value);

	switch (port) {
		case 0x3C4: return; // TODO -- Display stuff
		case 0x3C5: return; // TODO -- Display stuff
		case 0x3CE: return; // TODO -- VGA Register stuff
		case 0x3CF: return; // TODO -- VGA Register stuff
		case 0x3D4: return; // TODO -- Colour Display stuff
		case 0x3D5: return; // TODO -- Colour Display stuff

		default:
			fprintf(stderr, "[EMU] [ OUTW:%02X ] Not Yet Implemented\n", port);
			bios_uninit(1);
			return;
	}
}

/* OUTS */

void emu_outsb()
{
	emu_outb(emu_dx.x, emu_get_memory8(emu_es, emu_di, 0));
	if (emu_flags.df) emu_di -= 1; else emu_di += 1;
}
void emu_outsw()
{
	emu_outw(emu_dx.x, emu_get_memory16(emu_es, emu_di, 0));
	if (emu_flags.df) emu_di -= 2; else emu_di += 2;
}

/* SYSCALL */

void emu_syscall(uint8 value) {
	switch (value) {
		case 0x08: { // TIMER
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT08 ] TIMER\n");
			/* Increase the counter */
			emu_get_memory32(BIOS_MEMORY_PAGE, 0, BIOS_COUNTER)++;

			/* Preserve a few registers over INT1C calls */
			uint16 old_ds = emu_ds;
			uint16 old_dx = emu_dx.x;
			uint16 old_ax = emu_ax.x;
			emu_hard_int(0x1C);
			emu_ds = old_ds;
			emu_dx.x = old_dx;
			emu_ax.x = old_ax;
		} break;

		case 0x09: // KEYBOARD SERVICES
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT09 ] KEYBOARD SERVICES\n");
			/* This is for the user to handle */
			break;

		case 0x10: // VIDEO SERVICES
			emu_int10();
			break;

		case 0x11: // BIOS FLAGS
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT11 ] BIOS FLAGS\n");
			emu_ax.x = emu_get_memory16(BIOS_MEMORY_PAGE, BIOS_EQUIPEMENT, 0);
			break;

		case 0x12: // MEMORY SIZE
			emu_ax.x = 640;
			break;

		case 0x13: // DISKETTE SERVICE
			emu_int13();
			break;

		case 0x15: // SYSTEM SERVICES
			emu_int15();
			break;

		case 0x16: // KEYBOARD SERVICE
			emu_int16();
			break;

		case 0x1A: // CLOCK SERVICES
			emu_int1a();
			break;

		case 0x1C: // USER TIMER TICK
			/* This is for the user to handle */
			break;

		case 0x20: // TERMINATE APPLICATION
			fprintf(stderr, "[EMU] INT 0x20 application termination\n");
			bios_uninit(0);
			break;

		case 0x21: // DOS SERVICES
			emu_int21();
			break;

		case 0x2F: // DOS MULTIPLEX
			emu_int2f();
			break;

		case 0x33: // MOUSE
			emu_int33();
			break;

		default:
			fprintf(stderr, "[EMU] [ INT%02X ] Not Yet Implemented\n", value);
			bios_uninit(1);
			break;
	}

	/* Enable interrupt again, and push new flags on the stack, so popf() reads the new values */
	emu_flags.inf = 1;
	emu_get_memory16(emu_ss, emu_sp, 4) = emu_flags.all;
}

void emu_halt()
{
	fprintf(stderr, "[EMU] HALT called, closing application.\n");
	bios_uninit(1);
}
