/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"

/* XCHG */

void emu_xchgb(uint8 *val1, uint8 *val2)
{
	uint8 temp = *val1;
	*val1 = *val2;
	*val2 = temp;
}

void emu_xchgw(uint16 *val1, uint16 *val2)
{
	uint16 temp = *val1;
	*val1 = *val2;
	*val2 = temp;
}

/* LFP */

void emu_lfp(uint16 *seg, uint16 *reg, uint16 *data)
{
	*reg = *data;
	*seg = *(data + 1);
}

/* MOV */

void emu_movb(uint8 *dest, uint8 src)   { *dest = src; }
void emu_movw(uint16 *dest, uint16 src) { *dest = src; }
void emu_movws(uint16 *dest, int8 src)  { *dest = src; }

/* MOVS */

void emu_movsb(uint16 seg)
{
	emu_get_memory8(emu_es, emu_di, 0) = emu_get_memory8(seg, emu_si, 0);
	if (emu_flags.df) emu_si -= 1; else emu_si += 1;
	if (emu_flags.df) emu_di -= 1; else emu_di += 1;
}

void emu_movsw(uint16 seg)
{
	emu_get_memory16(emu_es, emu_di, 0) = emu_get_memory16(seg, emu_si, 0);
	if (emu_flags.df) emu_si -= 2; else emu_si += 2;
	if (emu_flags.df) emu_di -= 2; else emu_di += 2;
}

/* LODS */

void emu_lodsb(uint16 seg)
{
	emu_al = emu_get_memory8(seg, emu_si, 0);
	if (emu_flags.df) emu_si -= 1; else emu_si += 1;
}

void emu_lodsw(uint16 seg)
{
	emu_ax = emu_get_memory16(seg, emu_si, 0);
	if (emu_flags.df) emu_si -= 2; else emu_si += 2;
}

/* STOS */

void emu_stosb()
{
	emu_get_memory8(emu_es, emu_di, 0) = emu_al;
	if (emu_flags.df) emu_di -= 1; else emu_di += 1;
}

void emu_stosw()
{
	emu_get_memory16(emu_es, emu_di, 0) = emu_ax;
	if (emu_flags.df) emu_di -= 2; else emu_di += 2;
}

/* CMPS */

void emu_cmpsb(uint16 seg)
{
	emu_cmpb(&emu_get_memory8(seg, emu_si, 0), emu_get_memory8(emu_es, emu_di, 0));
	if (emu_flags.df) emu_si -= 1; else emu_si += 1;
	if (emu_flags.df) emu_di -= 1; else emu_di += 1;
}

void emu_cmpsw(uint16 seg)
{
	emu_cmpw(&emu_get_memory16(seg, emu_si, 0), emu_get_memory16(emu_es, emu_di, 0));
	if (emu_flags.df) emu_si -= 2; else emu_si += 2;
	if (emu_flags.df) emu_di -= 2; else emu_di += 2;
}

/* SCAS */

void emu_scasb()
{
	emu_cmpb(&emu_al, emu_get_memory8(emu_es, emu_di, 0));
	if (emu_flags.df) emu_di -= 1; else emu_di += 1;
}

void emu_scasw()
{
	emu_cmpw(&emu_ax, emu_get_memory16(emu_es, emu_di, 0));
	if (emu_flags.df) emu_di -= 2; else emu_di += 2;
}

/* CWD */

void emu_cwd()
{
	emu_dx = ((emu_ax & 0x8000) == 0) ? 0 : 0xFFFF;
}
