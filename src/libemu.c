#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"

uint16 emu_cs = 0, emu_ip = 0;
uint16 emu_ss = 0, emu_sp = 0;
uint16 emu_si = 0, emu_di = 0;
uint16 emu_ds = 0, emu_es = 0;
uint16 emu_bp = 0;
reg emu_ax = { { 0, 0 } };
reg emu_bx = { { 0, 0 } };
reg emu_cx = { { 0, 0 } };
reg emu_dx = { { 0, 0 } };
flags emu_flags = { { 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }, };

uint8 emu_memory[1024 * 1024];

uint8 emu_debug_int = 0;
uint32 emu_deep = 0;
uint8 emu_overlay = 0;

void emu_make_crash_dump()
{
	fprintf(stderr, " Creating crash-dump ...\n");

	FILE *fo = fopen("memory/crash.bin", "wb");

	{
		uint8 values[5];
		values[0] = 'E';
		values[1] = 'M';
		values[2] = 'U';
		values[3] = 0x17;
		values[4] = 4; // Version
		fwrite(values, sizeof(uint8), 5, fo);
	}

	{
		uint16 values[20];
		values[0]  = emu_cs;
		values[1]  = emu_ip;
		values[2]  = emu_ax.x;
		values[3]  = emu_bx.x;
		values[4]  = emu_cx.x;
		values[5]  = emu_dx.x;
		values[6]  = emu_si;
		values[7]  = emu_di;
		values[8]  = emu_bp;
		values[9]  = emu_sp;
		values[10] = emu_ds;
		values[11] = emu_es;
		values[12] = emu_ss;
		values[13] = emu_flags.all;
		values[14] = emu_deep;
		values[15] = emu_last_cs;
		values[16] = emu_last_ip;
		values[17] = emu_last_crc;
		values[18] = emu_last_length;
		values[19] = emu_overlay;
		fwrite(values, sizeof(uint16), 20, fo);
	}

	fwrite(&emu_get_memory8(0, 0, 0), 1024 * 1024, 1, fo);
	fclose(fo);

	fprintf(stderr, " Crash-dump in 'memory/crash.bin'\n");
}

void emu_init(int argc, char **argv)
{
	char memory_file[1024];
	if (argc == 2) {
		strcpy(memory_file, argv[1]);
	} else {
		strcpy(memory_file, "memory/start.bin");
	}

	/* Load the memory */
	FILE *fp = fopen(memory_file, "rb");
	if (fp == NULL) {
		fprintf(stderr, "[EMU] Failed to load memory file '%s'\n", memory_file);
		exit(1);
	}

	{
		/* Check the header */
		uint8 values[5];
		fread(values, sizeof(uint8), 5, fp);
		if (values[0] != 'E' || values[1] != 'M' || values[2] != 'U' || values[3] != 0x17) {
			fprintf(stderr, "[EMU] Not a valid memory file\n");
			exit(1);
		}
		if (values[4] != 4) {
			fprintf(stderr, "[EMU] Memory file is of wrong version and can't be loaded\n");
			exit(1);
		}
	}

	{
		/* Load the registers */
		uint16 values[20];
		fread(values, sizeof(uint16), 20, fp);
		emu_cs        = values[0];
		emu_ip        = values[1];
		emu_ax.x      = values[2];
		emu_bx.x      = values[3];
		emu_cx.x      = values[4];
		emu_dx.x      = values[5];
		emu_si        = values[6];
		emu_di        = values[7];
		emu_bp        = values[8];
		emu_sp        = values[9];
		emu_ds        = values[10];
		emu_es        = values[11];
		emu_ss        = values[12];
		emu_flags.all = values[13];
		emu_deep      = values[14];
		emu_last_cs   = values[15];
		emu_last_ip   = values[16];
		emu_last_crc  = values[17];
		emu_last_length = values[18];
		emu_overlay   = values[19];
	}

	/* Load the memory */
	fread(emu_memory, 1024 * 1024, 1, fp);

	/* Correct for the type we emulate */
#if EMULATE_386
	emu_flags.iopl = 3;
	emu_flags.nt   = 1;
	emu_flags.res4 = 0;
#elif EMULATE_286
	emu_flags.iopl = 0;
	emu_flags.nt   = 0;
	emu_flags.res4 = 0;
#else
	emu_flags.iopl = 3;
	emu_flags.nt   = 1;
	emu_flags.res4 = 1;
#endif
	emu_flags.res1 = 1;

	bios_init();
}

void trace(char *prefix, char *segment, uint8 type, uint16 segmentValue, uint16 registerOffset, uint16 offset, uint32 nothing)
{
	char memory[64];
	if (type == 0) memory[0] = '\0';
	else if (type == 1) sprintf(memory, "%s:[%04X]=%04X", segment, offset, emu_get_memory16(segmentValue, 0, offset));
	else sprintf(memory, "%s:[%04X]=%04X", segment, (uint16)(offset + registerOffset), emu_get_memory16(segmentValue, registerOffset, offset));
	fprintf(stderr, "%-47s%-23sEAX:%08X EBX:%08X ECX:%08X EDX:%08X ESI:%08X EDI:%08X EBP:%08X ESP:%08X DS:%04X ES:%04X FS:%04X GS:%04X SS:%04X CF:%d ZF:%d SF:%d OF:%d AF:%d PF:%d IF:%d\n", prefix, memory, emu_ax.x, emu_bx.x, emu_cx.x, emu_dx.x, emu_si, emu_di, emu_bp, emu_sp, emu_ds, emu_es, 0, 0, emu_ss, emu_flags.cf, emu_flags.zf, emu_flags.sf, emu_flags.of, emu_flags.af, emu_flags.pf, emu_flags.inf);
}

void debug()
{
	fprintf(stderr, "DEEP=%d\n", emu_deep);
	fprintf(stderr, "AX=%04X SI=%04X DS=%04X\n", emu_ax.x, emu_si, emu_ds);
	fprintf(stderr, "BX=%04X DI=%04X ES=%04X\n", emu_bx.x, emu_di, emu_es);
	fprintf(stderr, "CX=%04X CS=%04X IP=%04X BP=%04X\n", emu_cx.x, emu_cs, emu_ip, emu_bp);
	fprintf(stderr, "DX=%04X SP=%04X SS=%04X\n", emu_dx.x, emu_sp, emu_ss);
	fprintf(stderr, "C%d Z%d S%d O%d A%d P%d D%d I%d T%d\n", emu_flags.cf, emu_flags.zf, emu_flags.sf, emu_flags.of, emu_flags.af, emu_flags.pf, emu_flags.df, emu_flags.inf, emu_flags.tp);
}

void emu_data_monitor(uint16 s, uint16 o, uint16 d, int l)
{
	/* Show memory access */
	static int last_m = 0x0;
	static int last_l = 4;

	uint32 m = (s << 4) + ((o + d) & 0xFFFF);

	if (m > 0xF0000) {
		uint32 v = emu_memory[m];
		if (l == 2) v = v + (emu_memory[m + 1] << 8);
		if (l == 4) v = v + (emu_memory[m + 2] << 16) + (emu_memory[m + 3] << 24);
		fprintf(stderr, "[EMU] MEMORY ACCESS: 0x%X; VALUE: %08X\n", m, v);
	}
	if (last_m > 0xF0000) {
		uint32 v = emu_memory[last_m];
		if (last_l == 2) v = v + (emu_memory[last_m + 1] << 8);
		if (last_l == 4) v = v + (emu_memory[last_m + 2] << 16) + (emu_memory[last_m + 3] << 24);
		fprintf(stderr, "[EMU] PAST MEMORY ACCESS: 0x%X; VALUE: %08X\n", last_m, v);
	}

	last_m = m;
	last_l = l;
}
