/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"
#include "pic.h"
#include "int10.h"

typedef struct HardLink {
	uint16 cs;
	uint16 ip;
	void (*proc)();
} HardLink;

typedef struct HardLinkList {
	uint8 count;
	uint8 size;
	HardLink *entries;
} HardLinkList;

uint16 emu_last_ip = 0x0;
uint16 emu_last_cs = 0x0;
uint16 emu_last_length = 0x0;
uint16 emu_last_crc = 0x0;

static HardLinkList emu_hard_links = { 0, 0, NULL };

void emu_unknown_call()
{
	pic_suspend();
	emu_int10_uninit(0);

	fprintf(stderr, "Program Termination: jumped to not decompiled code.\n");
	fprintf(stderr, " The jump appears to originate from %04X:%04X.\n", emu_cs, emu_ip);
	fprintf(stderr, " Please send the file 'memory/crash.bin' to upstream developers.\n");

	bios_uninit(7);
	return;
}

void emu_call2(char *filename, int lineno)
{
	pic_suspend();
	emu_int10_uninit(0);

	fprintf(stderr, "Program Termination: jumped to %04X:%04X, which is not decompiled.\n", emu_cs, emu_ip);
	fprintf(stderr, " The jump was triggered at %s:%d\n", filename, lineno);
	fprintf(stderr, " The jump appears to originate from %04X:%04X.\n", emu_last_cs, emu_last_ip);
	fprintf(stderr, " Please send the file 'memory/crash.bin' to upstream developers.\n");

	bios_uninit(7);
	return;
}

void emu_calli(int16 add_ip, uint16 ret_ip, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_push(ret_ip);
	emu_ip = (ret_ip + add_ip) & 0xFFFF;

	emu_deep++;
}

void emu_calln(uint16 new_ip, uint16 ret_ip, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_push(ret_ip);
	emu_ip = new_ip;

	emu_deep++;
}

void emu_callf(uint16 new_cs, uint16 new_ip, uint16 ret_ip, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_push(emu_cs);
	emu_push(ret_ip);
	emu_ip = new_ip;
	emu_cs = new_cs;

	emu_deep++;
}

void emu_int(uint8 interrupt, uint16 ret_ip, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_pushf();
	emu_push(emu_cs);
	emu_push(ret_ip);
	emu_ip = emu_get_memory16(0, 0, interrupt * 4 + 0);
	emu_cs = emu_get_memory16(0, 0, interrupt * 4 + 2);
	emu_flags.tp = 0;
	emu_flags.inf = 0;

	emu_deep++;
}

void emu_hard_link(uint16 cs, uint16 ip, void (*proc)())
{
	if (emu_hard_links.count == emu_hard_links.size) {
		emu_hard_links.size += 4;
		emu_hard_links.entries = (HardLink *)realloc(emu_hard_links.entries, sizeof(HardLink) * emu_hard_links.size);
	}
	HardLink hl = { cs, ip, proc };
	emu_hard_links.entries[emu_hard_links.count++] = hl;
}

void emu_prepare_fuzzy(uint16 *ret_cs, uint16 *ret_ip)
{
}
void emu_check_fuzzy(uint16 ret_cs, uint16 ret_ip, uint16 ret_length, uint16 ret_crc)
{
}

void emu_hard_jump_recursive(uint16 deep)
{
	while (emu_deep == deep) {
		uint16 odeep  = emu_deep;
		uint16 ret_cs = emu_cs;

		emu_call();
		if (emu_deep == odeep) continue; // Normal jumps

		if (emu_deep > odeep) {
			/* We go up one level. Find out to what position we should return */
			uint16 ret_ip     = emu_get_memory16(emu_ss, 0, emu_sp);
			uint16 ret_length = emu_last_length;
			uint16 ret_crc    = emu_last_crc;

			/* Prepare fuzzy-checks */
			emu_prepare_fuzzy(&ret_cs, &ret_ip);

			/* Call the next recursive depth */
			emu_hard_jump_recursive(emu_deep);

			/* We go down one level. See if it was a normal return */
			emu_check_fuzzy(ret_cs, ret_ip, ret_length, ret_crc);
		}
	}
}

void emu_hard_jump(uint16 new_cs, uint16 new_ip)
{
	uint16 ret_cs = emu_cs;
	uint16 ret_ip = emu_ip;
	emu_ip = new_ip;
	emu_cs = new_cs;

	uint16 deep = emu_deep;
	emu_deep++;

	/* Call the function */
	int i;
	for (i = 0; i < emu_hard_links.count; i++) {
		if (emu_hard_links.entries[i].cs == emu_cs && emu_hard_links.entries[i].ip == emu_ip) {
			emu_hard_links.entries[i].proc();
			if (emu_cs == ret_cs && emu_ip == ret_ip) { emu_deep--; return; }
			break;
		}
	}
	while (emu_deep > deep || deep == 0) emu_hard_jump_recursive(emu_deep);
}

void emu_hard_call(uint16 new_cs, uint16 new_ip)
{
	uint16 ret_last_ip = emu_last_ip;
	uint16 ret_last_cs = emu_last_cs;
	uint16 ret_last_length = emu_last_length;
	uint16 ret_last_crc = emu_last_crc;
	emu_last_ip = 0x0;
	emu_last_cs = 0x0;
	emu_last_length = 0x0;
	emu_last_crc = 0x0;

	uint16 ret_cs = emu_cs;
	uint16 ret_ip = emu_ip;
	emu_push(emu_cs);
	emu_push(ret_ip);

	emu_hard_jump(new_cs, new_ip);

	/* Even if a return from an interrupt which is called from the
	 *  hardware gives another set of cs:ip, we ignore it and put in
	 *  the original, to prevent logic problems */
	emu_cs = ret_cs;
	emu_ip = ret_ip;
	emu_last_crc = ret_last_crc;
	emu_last_length = ret_last_length;
	emu_last_cs = ret_last_cs;
	emu_last_ip = ret_last_ip;
}

void emu_hard_int(uint8 interrupt)
{
	uint16 ret_last_ip = emu_last_ip;
	uint16 ret_last_cs = emu_last_cs;
	uint16 ret_last_length = emu_last_length;
	uint16 ret_last_crc = emu_last_crc;
	emu_last_ip = 0x0;
	emu_last_cs = 0x0;
	emu_last_length = 0x0;
	emu_last_crc = 0x0;

	uint16 ret_cs = emu_cs;
	uint16 ret_ip = emu_ip;
	emu_pushf();
	emu_push(emu_cs);
	emu_push(emu_ip);
	emu_flags.inf = 0;

	emu_hard_jump(emu_get_memory16(0, 0, interrupt * 4 + 2), emu_get_memory16(0, 0, interrupt * 4 + 0));

	/* Even if a return from an interrupt which is called from the
	 *  hardware gives another set of cs:ip, we ignore it and put in
	 *  the original, to prevent logic problems */
	emu_cs = ret_cs;
	emu_ip = ret_ip;
	emu_last_crc = ret_last_crc;
	emu_last_length = ret_last_length;
	emu_last_cs = ret_last_cs;
	emu_last_ip = ret_last_ip;
}

void emu_jmpi(int16 add_ip, uint16 ret_ip, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_ip = (ret_ip + add_ip) & 0xFFFF;
}

void emu_jmpn(uint16 new_ip, uint16 ret_ip, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_ip = new_ip;
}

void emu_jmpf(uint16 new_cs, uint16 new_ip, uint16 ret_ip, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_ip = new_ip;
	emu_cs = new_cs;
}

void emu_ret(uint8 stack_clean, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_pop(&emu_ip);
	emu_sp += stack_clean;

	emu_deep--;
}

void emu_retf(uint8 stack_clean, uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_pop(&emu_ip);
	emu_pop(&emu_cs);
	emu_sp += stack_clean;

	emu_deep--;
}

void emu_reti(uint32 from_ip)
{
	emu_last_ip = from_ip;
	emu_last_cs = emu_cs;

	emu_pop(&emu_ip);
	emu_pop(&emu_cs);
	emu_popf();

	emu_deep--;
}
