/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

typedef struct XMSBlock {
	uint32 size;
	void *memory;
} XMSBlock;

static XMSBlock s_xms_handlers[10];

void xms_init()
{
	memset(s_xms_handlers, 0, sizeof(s_xms_handlers));
}

void xms_uninit()
{
	uint16 i;

	for (i = 0; i < 10; i++) {
		if (s_xms_handlers[i].size == 0) continue;
		free(s_xms_handlers[i].memory);
	}
}

void emu_xms()
{
	switch (emu_ah) {
		case 0x00: /* GET VERSION NUMBER */
		{          /* Return: AX -> XMS version number, BX -> Internal revision number, DX -> 1 if HMA, 0 otherwise */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ XMS:00 ] GET VERSION NUMBER\n");

			emu_ax = 0x0300;
			emu_bx = 0x0301;
			emu_dx = 0; /* No HMA */
		} return;

		case 0x08: /* QUERY FREE EXTENDED MEMORY */
		{          /* Return: AX -> largest free block (in kb), DX -> total amount of free kb */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ XMS:08 ] QUERY FREE EXTENDED MEMORY\n");

			emu_ax = 0x0500;
			emu_dx = 0x4000;
			emu_bx = 0;
		} return;

		case 0x09: /* ALLOCATED EXTENDED MEMORY BLOCK, DX -> amount of kb requested */
		{          /* Return: AX -> 1 if the block is alloacted, 0 otherwise, DX -> handler for the block */
			uint16 i;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ XMS:09 ] ALLOCATED EXTENDED MEMORY BLOCK %d kilobytes\n", emu_dx);

			emu_ax = 0;
			emu_bx = 0;

			for (i = 0; i < 10; i++) {
				if (s_xms_handlers[i].size == 0) break;
			}
			if (i == 10) {
				emu_bl = 0xA1; /* OUT OF HANDLERS */
				return;
			}

			s_xms_handlers[i].size = emu_dx * 1024;
			s_xms_handlers[i].memory = calloc(emu_dx, 1024);

			emu_ax = 1;
			emu_dx = i + 1;
		} return;

		case 0x0A: /* FREE EXTENDED MEMORY BLOCK, DX -> handler of the block */
		{          /* Return: AX -> 1 if the block is freed, 0 otherwise */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ XMS:0A ] FREE EXTENDED MEMORY BLOCK '%d'\n", emu_dx);

			emu_ax = 0;

			if (emu_dx > 10 || emu_dx == 0 || s_xms_handlers[emu_dx - 1].size == 0) {
				emu_bx = 0xA2; /* INVALID HANDLE */
				return;
			}

			free(s_xms_handlers[emu_dx - 1].memory);
			s_xms_handlers[emu_dx - 1].size = 0;
			s_xms_handlers[emu_dx - 1].memory = NULL;

			emu_ax = 1;
		} return;

		case 0x0B: /* MOVE EXTENDED MEMORY BLOCK, DS:SI -> pointer to EMMS */
		{          /* Return: AX -> 1 if the block is moved, 0 otherwise */
			uint32 length       = emu_get_memory32(emu_ds, emu_si, 0);
			uint16 sourceHandle = emu_get_memory16(emu_ds, emu_si, 4);
			uint32 sourceOffset = emu_get_memory32(emu_ds, emu_si, 6);
			uint16 destHandle   = emu_get_memory16(emu_ds, emu_si, 10);
			uint32 destOffset   = emu_get_memory32(emu_ds, emu_si, 12);
			void *source;
			void *dest;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ XMS:0A ] MOVE EXTENDED MEMORY BLOCK '%04X:%04X'\n", emu_ds, emu_si);

			emu_ax = 0;
			emu_bx = 0;

			if (sourceHandle == 0) {
				source = (void *)&emu_get_memory8(sourceOffset >> 16, sourceOffset & 0xFFFF, 0);
			} else {
				if (sourceHandle > 10 || sourceHandle == 0 || s_xms_handlers[sourceHandle - 1].size == 0) {
					emu_bx = 0xA3; /* INVALID SOURCE HANDLE */
					return;
				}
				if (sourceOffset + length > s_xms_handlers[sourceHandle - 1].size) {
					emu_bx = 0xA4; /* INVALID SOURCE OFFSET */
					return;
				}
				source = (void *)((uint8 *)s_xms_handlers[sourceHandle - 1].memory + sourceOffset);
			}
			if (destHandle == 0) {
				dest = (void *)&emu_get_memory8(destOffset >> 16, destOffset & 0xFFFF, 0);
			} else {
				if (destHandle > 10 || destHandle == 0 || s_xms_handlers[destHandle - 1].size == 0) {
					emu_bx = 0xA5; /* INVALID DEST HANDLE */
					return;
				}
				if (destOffset + length > s_xms_handlers[destHandle - 1].size) {
					emu_bx = 0xA6; /* INVALID DEST OFFSET */
					return;
				}
				dest = (void *)((uint8 *)s_xms_handlers[destHandle - 1].memory + destOffset);
			}

			memmove(dest, source, length);
			emu_ax = 1;
		} return;

		case 0x0C: /* LOCK EXTENDED MEMORY BLOCK, DX -> handler of the block */
		{          /* Return: AX -> 1 if the block is locked, 0 otherwise */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ XMS:0C ] LOCK EXTENDED MEMORY BLOCK '%d'\n", emu_dx);

			emu_ax = 0;

			if (emu_dx > 10 || emu_dx == 0 || s_xms_handlers[emu_dx - 1].size == 0) {
				emu_bx = 0xA2; /* INVALID HANDLE */
				return;
			}

			emu_ax = 1;
			/* As real 32bit address we return a value bigger than 1M.
			 *  A multipler of the handler with 1M should be sufficient
			 *  now as no block will be bigger than 1M, and no sane
			 *  application should use the value directly.
			 * NOTE: the emu_dx value seems to be read BE (or at
			 *  least by Dune2, so that is why it is shifted with
			 *  4. Giving 0x1203 makes Dune2 at least think it is
			 *  in the lower 1M memory, and it tries to reach
			 *  3120:0000 instead of the upper memory. */
			emu_bx = 0x0000;
			emu_dx = emu_dx << 4;
		} return;

		case 0x0D: /* UNLOCK EXTENDED MEMORY BLOCK, DX -> handler of the block */
		{          /* Return: AX -> 1 if the block is unlocked, 0 otherwise */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ XMS:0D ] UNLOCK EXTENDED MEMORY BLOCK '%d'\n", emu_dx);

			emu_ax = 0;

			if (emu_dx > 10 || emu_dx == 0 || s_xms_handlers[emu_dx - 1].size == 0) {
				emu_bx = 0xA2; /* INVALID HANDLE */
				return;
			}

			emu_ax = 1;
		} return;

		default:
			fprintf(stderr, "[EMU] [ XMS:%02X ] Not Yet Implemented\n", emu_ah);
			bios_uninit(1);
	}
}
