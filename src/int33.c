/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

MSVC_PACKED_BEGIN
typedef struct Mouse {
	PACK uint16 pos_x;
	PACK uint16 pos_y;

	PACK  int16 dx;
	PACK  int16 dy;

	PACK uint16 min_x;
	PACK uint16 min_y;
	PACK uint16 max_x;
	PACK uint16 max_y;

	PACK uint8  left_button;
	PACK uint8  right_button;

	PACK uint16 callback_cs;
	PACK uint16 callback_ip;
	PACK uint8  callback_mask;

	PACK uint8  unused[1];
} GCC_PACKED Mouse;
MSVC_PACKED_END
assert_compile(sizeof(Mouse) == 24);

static Mouse *emu_mouse;

void emu_int33_init()
{
	emu_mouse = (Mouse *)&emu_get_memory8(MOUSE_MEMORY_PAGE, 0, 0);
}

void emu_mouse_callback(uint8 reason)
{
	uint16 old_ax = emu_ax;
	uint16 old_bx = emu_bx;
	uint16 old_cx = emu_cx;
	uint16 old_dx = emu_dx;
	uint16 old_ds = emu_ds;
	uint16 old_es = emu_es;
	uint16 old_di = emu_di;
	uint16 old_si = emu_si;

	if (emu_mouse[0].callback_ip == 0x0 && emu_mouse[0].callback_cs == 0x0) return;

	emu_ax = reason;
	emu_bx = (emu_mouse[0].left_button ? 0x1 : 0x0) | (emu_mouse[0].right_button ? 0x2 : 0x0);
	emu_cx = emu_mouse[0].pos_x;
	emu_dx = emu_mouse[0].pos_y;
	emu_di   = emu_mouse[0].dx;
	emu_si   = emu_mouse[0].dy;
	emu_ds   = 0;

	emu_hard_call(emu_mouse[0].callback_cs, emu_mouse[0].callback_ip);

	emu_ax = old_ax;
	emu_bx = old_bx;
	emu_cx = old_cx;
	emu_dx = old_dx;
	emu_ds   = old_ds;
	emu_es   = old_es;
	emu_di   = old_di;
	emu_si   = old_si;
}

void emu_mouse_change_position(uint16 x, uint16 y)
{
	if (emu_mouse[0].min_x != 0 && x < emu_mouse[0].min_x) x = emu_mouse[0].min_x;
	if (emu_mouse[0].max_x != 0 && x > emu_mouse[0].max_x) x = emu_mouse[0].max_x;
	if (emu_mouse[0].min_y != 0 && y < emu_mouse[0].min_y) y = emu_mouse[0].min_y;
	if (emu_mouse[0].max_y != 0 && y > emu_mouse[0].max_y) y = emu_mouse[0].max_y;

	emu_mouse[0].dx = x - emu_mouse[0].pos_x;
	emu_mouse[0].dy = y - emu_mouse[0].pos_y;
	emu_mouse[0].pos_x = x;
	emu_mouse[0].pos_y = y;
	if ((emu_mouse[0].callback_mask & 0x01) == 0x01) emu_mouse_callback(0x01);
}

void emu_mouse_change_button(uint8 left, uint8 press)
{
	uint8 reason;
	if (left != 0) {
		emu_mouse[0].left_button = press;
		reason = press ? 0x02 : 0x04;
	} else {
		emu_mouse[0].right_button = press;
		reason = press ? 0x08 : 0x10;
	}

	if ((emu_mouse[0].callback_mask & reason) == reason) emu_mouse_callback(reason);
}

void emu_int33()
{
	switch (emu_ax) {
		case 0x00: /* MOUSE INSTALLED FLAG */
		{          /* Return: AX -> 0x0000 (not installed), 0xFFFF, (installed) */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT33:00 ] MOUSE INSTALLED FLAG\n");
			emu_ax = 0xFFFF;
		} return;

		case 0x03: /* GET MOUSE POSITION AND BUTTON STATUS */
		{          /* Return: BX -> button, CX -> horizontal, DX -> vertical */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT33:03 ] GET MOUSE POSITION AND BUTTON STATUS\n");
			emu_bx = (emu_mouse[0].left_button ? 0x1 : 0x0) | (emu_mouse[0].right_button ? 0x2: 0x0);
			emu_cx = emu_mouse[0].pos_x;
			emu_dx = emu_mouse[0].pos_y;
		} return;

		case 0x04: /* SET MOUSE POSITION, CX -> horizontal, DX -> vertical */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT33:04 ] SET MOUSE POSITION to %dx%d\n", emu_cx, emu_dx);

			emu_mouse[0].pos_x = emu_cx;
			emu_mouse[0].pos_y = emu_dx;
		} return;

		case 0x07: /* SET MOUSE HORIZONTAL MIN/MAX, CX -> minimum, DX ->maximum */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT33:07 ] SET MOUSE HORIZONTAL MIN/MAX to %d - %d\n", emu_cx, emu_dx);

			emu_mouse[0].min_x = emu_cx;
			emu_mouse[0].max_x = emu_dx;
		} return;

		case 0x08: /* SET MOUSE VERTICAL MIN/MAX, CX -> minimum, DX -> maximum */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT33:08 ] SET MOUSE VERTICAL MIN/MAX to %d - %d\n", emu_cx, emu_dx);

			emu_mouse[0].min_y = emu_cx;
			emu_mouse[0].max_y = emu_dx;
		} return;

		case 0x0C: /* SET USER CALLBACK, CX -> mask, ES:DX -> callback */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT33:0C ] SET USER CALLBACK to %04X:%04X\n", emu_es, emu_dx);

			emu_mouse[0].callback_cs = emu_es;
			emu_mouse[0].callback_ip = emu_dx;
			emu_mouse[0].callback_mask = emu_cl;
		} return;

		case 0x24: /* GET INFORMATION */
		{          /* Return: BH -> major, BL -> minor, CH -> type, CL -> IRQ */
			emu_bh = 8;
			emu_bl = 5;
			emu_ch = 4;
			emu_cl = 0;
		} return;

		default:
			fprintf(stderr, "[EMU] [ INT33:%02X ] Not Yet Implemented\n", emu_ax);
			bios_uninit(1);
	}
}
