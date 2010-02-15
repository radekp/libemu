/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "mpu.h"

static uint8 mpu_data = 0x00;
static uint8 mpu_ctrl = 0x80;

/* MPU DATA */
uint8 emu_io_read_330()
{
	uint8 ret;

	ret = mpu_data;
	mpu_data = 0x00;

	mpu_ctrl &= 0x3F;
	mpu_ctrl |= 0x80;

	return ret;
}

void emu_io_write_330(uint8 value)
{
	static bool sysex = false;
	static uint8 need = 0;
	static uint32 msg = 0;
	static uint8 *pmsg = NULL;

	if (emu_debug_int) fprintf(stderr, "[EMU] [ OUTB:0x330] VALUE: 0x%02X\n", value);

	if (((need == 0) != ((value & 0x80) != 0)) && (!sysex || (value == 0xF7))) {
		fprintf(stderr, "[EMU] Invalid data received: %02X (ignoring)\n", value);
		return;
	}

	mpu_ctrl |= 0x40;

	if (value < 0x80) {
		if (!sysex) {
			*(pmsg++) = value;
			if (--need == 0) {
				mpu_send(msg);
			}
		}
	} else if (value < 0xC0) {
		msg = 0;
		pmsg = (uint8 *)&msg;
		*(pmsg++) = value;
		need = 2;
	} else if (value < 0xE0) {
		msg = 0;
		pmsg = (uint8 *)&msg;
		*(pmsg++) = value;
		need = 1;
	} else if (value < 0xF0) {
		msg = 0;
		pmsg = (uint8 *)&msg;
		*(pmsg++) = value;
		need = 2;
	} else if (value == 0xF0) {
		sysex = true;
	} else if (value == 0xF7) {
		if (sysex) sysex = false;
	}

	mpu_ctrl &= 0xBF;
}

/* MPU STATUS/COMMAND */
uint8 emu_io_read_331()
{
	return mpu_ctrl;
}

void emu_io_write_331(uint8 value)
{
	if (emu_debug_int) fprintf(stderr, "[EMU] [ OUTB:0x331] VALUE: 0x%02X\n", value);

	if (value == 0xFF) {
		mpu_reset();
		mpu_data = 0xFE;
		mpu_ctrl &= 0x3F;
		mpu_ctrl |= 0x40;
	}
}
