/* $Id$ */

#include <stdio.h>
#include "types.h"
#include "libemu.h"

/* BOUND */

void emu_boundb(uint8 reg, uint8 *bound) {
	if (reg < *bound || reg > *(bound + 1)) emu_hard_int(0x5);
}
void emu_boundw(uint16 reg, uint16 *bound) {
	if (reg < *bound || reg > *(bound + 1)) emu_hard_int(0x5);
}

/* ADD */

void emu_addb(uint8 *dest, uint8 val2) {
	uint8 val1 = *dest;

	*dest += val2;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (*dest < val1) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val1 ^ val2 ^ 0x80) & (*dest ^ val1)) >> 7;
}
void emu_addw(uint16 *dest, uint16 val2) {
	uint16 val1 = *dest;

	*dest += val2;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (*dest < val1) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val1 ^ val2 ^ 0x8000) & (*dest ^ val1)) >> 15;
}
void emu_addws(uint16 *dest, int8 val2) { emu_addw(dest, val2); }


/* ADC */

void emu_adcb(uint8 *dest, uint8 val2) {
	uint8 val1 = *dest;

	*dest += val2 + emu_flags.cf;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (*dest < val1 || (emu_flags.cf && (*dest) == val1)) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val1 ^ val2 ^ 0x80) & (*dest ^ val1)) >> 7;
}
void emu_adcw(uint16 *dest, uint16 val2) {
	uint16 val1 = *dest;

	*dest += val2 + emu_flags.cf;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (*dest < val1 || (emu_flags.cf && (*dest) == val1)) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val1 ^ val2 ^ 0x8000) & (*dest ^ val1)) >> 15;
}
void emu_adcws(uint16 *dest, int8 val2) { emu_adcw(dest, val2); }


/* SUB */

void emu_subb(uint8 *dest, uint8 val2) {
	uint8 val1 = *dest;

	*dest -= val2;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (val2 > val1) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val1 ^ val2) & (*dest ^ val1)) >> 7;
}
void emu_subw(uint16 *dest, uint16 val2) {
	uint16 val1 = *dest;

	*dest -= val2;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (val2 > val1) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val2 ^ val1) & (*dest ^ val1)) >> 15;
}
void emu_subws(uint16 *dest, int8 src) { emu_subw(dest, src); }


/* SBB */

void emu_sbbb(uint8 *dest, uint8 val2) {
	uint8 val1 = *dest;

	*dest -= val2 + emu_flags.cf;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (*dest > val1 || (emu_flags.cf && val2 == 0xFF)) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val1 ^ val2) & (*dest ^ val1)) >> 7;
}
void emu_sbbw(uint16 *dest, uint16 val2) {
	uint16 val1 = *dest;

	*dest -= val2 + emu_flags.cf;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = (*dest > val1 || (emu_flags.cf && val2 == 0xFFFF)) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ *dest) & 0x10) ? 1 : 0;
	emu_flags.of = ((val2 ^ val1) & (*dest ^ val1)) >> 15;
}
void emu_sbbws(uint16 *dest, int8 src) { emu_sbbw(dest, src); }


/* INC */

void emu_incb(uint8 *dest) {
	*dest += 1;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.af = ((*dest & 0x0F) == 0) ? 1 : 0;
	emu_flags.of = ((*dest) == 0x80) ? 1 : 0;
}

void emu_incw(uint16 *dest) {
	*dest += 1;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.af = ((*dest & 0x000F) == 0) ? 1 : 0;
	emu_flags.of = ((*dest) == 0x8000) ? 1 : 0;
}


/* DEC */

void emu_decb(uint8 *dest) {
	*dest -= 1;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.af = ((*dest & 0x0F) == 0x0F) ? 1 : 0;
	emu_flags.of = (*dest) == 0x7F ? 1 : 0;
}

void emu_decw(uint16 *dest) {
	*dest -= 1;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.af = ((*dest & 0x000F) == 0x000F) ? 1 : 0;
	emu_flags.of = (*dest) == 0x7FFF ? 1 : 0;
}


/* CMP */

void emu_cmpb(uint8 *dest, uint8 val2) {
	uint8 val1 = *dest;

	uint8 destT = *dest - val2;

	emu_flags_sfb(destT);
	emu_flags_zf (destT);
	emu_flags_pf (destT);
	emu_flags.cf = (val2 > val1) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ destT) & 0x10) ? 1 : 0;
	emu_flags.of = ((val1 ^ val2) & (destT ^ val1)) >> 7;
}
void emu_cmpw(uint16 *dest, uint16 val2) {
	uint16 val1 = *dest;

	uint16 destT = *dest - val2;

	emu_flags_sfw(destT);
	emu_flags_zf (destT);
	emu_flags_pf (destT);
	emu_flags.cf = (val2 > val1) ? 1 : 0;
	emu_flags.af = (((val1 ^ val2) ^ destT) & 0x10) ? 1 : 0;
	emu_flags.of = ((val2 ^ val1) & (destT ^ val1)) >> 15;
}
void emu_cmpws(uint16 *dest, int8 src) { emu_cmpw(dest, src); }


/* AND */

void emu_andb(uint8 *dest, uint8 val2) {
	*dest &= val2;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_andw(uint16 *dest, uint16 val2) {
	*dest &= val2;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_andws(uint16 *dest, int8 val2) { emu_andw(dest, val2); }


/* TEST */

void emu_testb(uint8 *dest, uint8 val2) {
	uint8 destT = *dest & val2;

	emu_flags_sfb(destT);
	emu_flags_zf (destT);
	emu_flags_pf (destT);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_testw(uint16 *dest, uint16 val2) {
	uint16 destT = *dest & val2;

	emu_flags_sfw(destT);
	emu_flags_zf (destT);
	emu_flags_pf (destT);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_testws(uint16 *dest, int8 val2) { emu_testw(dest, val2); }


/* OR */

void emu_orb(uint8 *dest, uint8 val2) {
	*dest |= val2;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_orw(uint16 *dest, uint16 val2) {
	*dest |= val2;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_orws(uint16 *dest, int8 val2) { emu_orw(dest, val2); }


/* XOR */

void emu_xorb(uint8 *dest, uint8 val2) {
	*dest ^= val2;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_xorw(uint16 *dest, uint16 val2) {
	*dest ^= val2;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.cf = 0;
	emu_flags.af = 0;
	emu_flags.of = 0;
}
void emu_xorws(uint16 *dest, int8 val2) { emu_xorw(dest, val2); }


/* NEG */

void emu_negb(uint8 *dest, uint8 val2) {
	emu_flags.of = (*dest == 0x80) ? 1 : 0;

	*dest = -val2;

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.af = (*dest & 0x0F) ? 1 : 0;
	emu_flags.cf = (val2 == 0) ? 0 : 1;
}
void emu_negw(uint16 *dest, uint16 val2) {
	emu_flags.of = (*dest == 0x8000) ? 1 : 0;

	*dest = -val2;

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.af = (*dest & 0x0F) ? 1 : 0;
	emu_flags.cf = (val2 == 0) ? 0 : 1;
}
void emu_negws(uint16 *dest, int8 val2) { emu_negw(dest, val2); }


/* NOT */

void emu_notb(uint8 *dest, uint8 val2) {
	*dest = ~val2;
}
void emu_notw(uint16 *dest, uint16 val2) {
	*dest = ~val2;
}
void emu_notws(uint16 *dest, int8 val2) { emu_notw(dest, val2); }


/* MUL */

void emu_mulb(uint8 *dest, uint8 val2) {
	uint16 res = emu_al * val2;

	emu_al = res & 0xFF;
	emu_ah = res >> 8;

	emu_flags.zf = (emu_al == 0) ? 1 : 0;
	emu_flags.cf = (emu_ah == 0) ? 0 : 1;
	emu_flags.of = (emu_ah == 0) ? 0 : 1;
}
void emu_mulw(uint16 *dest, uint16 val2) {
	uint32 res = emu_ax * val2;

	emu_ax = res & 0xFFFF;
	emu_dx = res >> 16;

	emu_flags.zf = (emu_ax == 0) ? 1 : 0;
	emu_flags.cf = (emu_dx == 0) ? 0 : 1;
	emu_flags.of = (emu_dx == 0) ? 0 : 1;
}
void emu_mulws(uint16 *dest, int8 src) { emu_mulw(dest, src); }


/* IMUL */

void emu_imulb(uint8 *dest, int8 val1, int8 val2) {
	int16 res = val1 * val2;
	*dest = res;

	emu_flags.cf = (res > 0xFF) ? 1 : 0;
	emu_flags.of = (res > 0xFF) ? 1 : 0;
}
void emu_imulw(uint16 *dest, int16 val1, int16 val2) {
	int32 res = val1 * val2;
	*dest = res;

	emu_flags.cf = (res > 0xFFFF) ? 1 : 0;
	emu_flags.of = (res > 0xFFFF) ? 1 : 0;
}
void emu_imulws(uint16 *dest, int16 val1, int8 val2) { emu_imulw(dest, val1, val2); }
void emu_imulub(uint8 *dest, int8 val) {
	int16 res = (int8)emu_al * val;

	emu_al = res;
	emu_ah = res >> 8;

	/* It depends on how you define 'sign extended' .. but this keeps it DosBox compatible */
#if 1
	emu_flags.cf = ((emu_ah == 0xFF && (emu_al & 0x80)) || (emu_ah == 0x00 && !(emu_al & 0x80))) ? 0 : 1;
	emu_flags.of = ((emu_ah == 0xFF && (emu_al & 0x80)) || (emu_ah == 0x00 && !(emu_al & 0x80))) ? 0 : 1;
#else
	emu_flags.cf = (emu_ah == 0) ? 1 : 0;
	emu_flags.of = (emu_ah == 0) ? 1 : 0;
#endif
}
void emu_imuluw(uint16 *dest, int16 val) {
	int32 res = (int16)emu_ax * val;

	emu_ax = res;
	emu_dx = res >> 16;

	/* It depends on how you define 'sign extended' .. but this keeps it DosBox compatible */
#if 1
	emu_flags.cf = ((emu_dx == 0xFFFF && (emu_ax & 0x8000)) || (emu_dx == 0x0000 && !(emu_ax & 0x8000))) ? 0 : 1;
	emu_flags.of = ((emu_dx == 0xFFFF && (emu_ax & 0x8000)) || (emu_dx == 0x0000 && !(emu_ax & 0x8000))) ? 0 : 1;
#else
	emu_flags.cf = (emu_dx == 0) ? 1 : 0;
	emu_flags.of = (emu_dx == 0) ? 1 : 0;
#endif
}


/* DIV */

void emu_divb(uint8 *dest, uint8 val2) {
	emu_al = emu_ax / val2;
	emu_ah = emu_ax % val2;
}
void emu_divw(uint16 *dest, uint16 val2) {
	int32 dividend = ((emu_dx << 16) + emu_ax);

	emu_ax = dividend / val2;
	emu_dx = dividend % val2;
}


/* IDIV */

void emu_idivb(uint8 *dest, int8 val2) {
	emu_al = (int)emu_ax / val2;
	emu_ah = (int)emu_ax % val2;
}
void emu_idivw(uint16 *dest, int16 val2) {
	int32 dividend = ((emu_dx << 16) + emu_ax);

	emu_ax = dividend / val2;
	emu_dx = dividend % val2;
}


/* SHL */

void emu_shlb(uint8 *dest, uint8 val2) {
	val2 = val2 & 0x1F;
	emu_flags.af = (val2 != 0) ? 1: 0;

	emu_flags.of = ((*dest) >> (7 - val2)) ^ ((*dest) >> 7); /* DosBox compatible */

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) >> 7;
		*dest <<= 1;
	}

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
/*	emu_flags.of = emu_flags.cf ^ ((*dest) >> 7); */
}
void emu_shlw(uint16 *dest, uint16 val2) {
	val2 = val2 & 0x1F;
	emu_flags.af = (val2 != 0) ? 1: 0;

	emu_flags.of = ((*dest) >> (15 - val2)) ^ ((*dest) >> 15); /* DosBox compatible */

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) >> 15;
		*dest <<= 1;
	}

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
/*	emu_flags.of = emu_flags.cf ^ ((*dest) >> 15); */
}
void emu_shlws(uint16 *dest, int8 val2) { emu_shlw(dest, val2); }


/* SHR */

void emu_shrb(uint8 *dest, uint8 val2) {
	val2 = val2 & 0x1F;
	emu_flags.af = (val2 != 0) ? 1: 0;

	emu_flags.of = (val2 == 1) ? ((*dest) >> 7) : 0;

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
	}

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
}
void emu_shrw(uint16 *dest, uint16 val2) {
	val2 = val2 & 0x1F;
	emu_flags.af = (val2 != 0) ? 1: 0;

	emu_flags.of = (val2 == 1) ? ((*dest) >> 15) : 0;

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
	}

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
}
void emu_shrws(uint16 *dest, int8 val2) { emu_shrw(dest, val2); }


/* SAR */

void emu_sarb(uint8 *dest, uint8 val2) {
	val2 = val2 & 0x1F;
	emu_flags.af = (val2 != 0) ? 1: 0;

	int8 sign = 0x0;
	if (((*dest) & 0x80) != 0) sign = 0x80;

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
		*dest |= sign;
	}

	emu_flags_sfb(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.of = 0;
}
void emu_sarw(uint16 *dest, uint16 val2) {
	val2 = val2 & 0x1F;
	emu_flags.af = (val2 != 0) ? 1: 0;

	uint16 sign = 0x0;
	if ((*dest) & 0x8000) sign = 0x8000;

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
		*dest |= sign;
	}

	emu_flags_sfw(*dest);
	emu_flags_zf (*dest);
	emu_flags_pf (*dest);
	emu_flags.of = 0;
}
void emu_sarws(uint16 *dest, int8 val2) { emu_sarw(dest, val2); }


/* ROL */

void emu_rolb(uint8 *dest, uint8 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) >> 7;
		*dest <<= 1;
		*dest += emu_flags.cf;
	}

	emu_flags.of = emu_flags.cf ^ ((*dest) >> 7);
}
void emu_rolw(uint16 *dest, uint16 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) >> 15;
		*dest <<= 1;
		*dest += emu_flags.cf;
	}

	emu_flags.of = emu_flags.cf ^ ((*dest) >> 15);
}
void emu_rolws(uint16 *dest, int8 val2) { emu_rolw(dest, val2); }


/* ROR */

void emu_rorb(uint8 *dest, uint8 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
		*dest += (emu_flags.cf << 7);
	}

	emu_flags.of = ((*dest) >> 7) ^ ((*dest) >> 6);
}
void emu_rorw(uint16 *dest, uint16 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
		*dest += (emu_flags.cf << 15);
	}

	emu_flags.of = ((*dest) >> 15) ^ ((*dest) >> 14);
}
void emu_rorws(uint16 *dest, int8 val2) { emu_rorw(dest, val2); }


/* RCL */

void emu_rclb(uint8 *dest, uint8 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		uint8 old_cf = emu_flags.cf;
		emu_flags.cf = (*dest) >> 7;
		*dest <<= 1;
		*dest += old_cf;
	}

	emu_flags.of = emu_flags.cf ^ ((*dest) >> 7);
}
void emu_rclw(uint16 *dest, uint16 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		uint8 old_cf = emu_flags.cf;
		emu_flags.cf = (*dest) >> 15;
		*dest <<= 1;
		*dest += old_cf;
	}

	emu_flags.of = emu_flags.cf ^ ((*dest) >> 15);
}
void emu_rclws(uint16 *dest, int8 val2) { emu_rclw(dest, val2); }


/* RCR */

void emu_rcrb(uint8 *dest, uint8 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		uint8 old_cf = emu_flags.cf;
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
		*dest += (old_cf << 7);
	}

	emu_flags.of = ((*dest) >> 7) ^ ((*dest) >> 6);
}
void emu_rcrw(uint16 *dest, uint16 val2) {
	val2 = val2 & 0x1F;
/*	emu_flags.af = (val2 != 0) ? 1: 0; */

	for (; val2 > 0; --val2) {
		uint8 old_cf = emu_flags.cf;
		emu_flags.cf = (*dest) & 0x1;
		*dest >>= 1;
		*dest += (old_cf << 15);
	}

	emu_flags.of = ((*dest) >> 15) ^ ((*dest) >> 14);
}
void emu_rcrws(uint16 *dest, int8 val2) { emu_rcrw(dest, val2); }


/* AAA */

void emu_aaa()
{
	emu_flags.sf = (emu_al >= 0x7A && emu_al <= 0xF9) ? 1 : 0;

	if ((emu_al & 0x0F) > 0x09 || emu_flags.af) {
		emu_ax += 0x0106;
		emu_flags.of = ((emu_al & 0xF0) == 0x70 && !emu_flags.af) ? 1 : 0;
		emu_flags.af = 1;
		emu_flags.cf = 1;
	} else {
		emu_flags.of = 0;
		emu_flags.af = 0;
		emu_flags.cf = 0;
	}

	emu_flags_zf(emu_al);
	emu_flags_pf(emu_al);

	/* DosBox compatible: do this at the end */
	emu_al &= 0x0F;
}


/* AAD */

void emu_aad()
{
	emu_al += emu_ah * 10;
	emu_ah = 0;

	emu_flags_sfb(emu_al);
	emu_flags_zf(emu_al);
	emu_flags_pf(emu_al);
	emu_flags.cf = 0;
	emu_flags.of = 0;
	emu_flags.af = 0;
}


/* AAM */

void emu_aam()
{
	emu_ah = emu_al / 10;
	emu_al = emu_al % 10;

	emu_flags_sfb(emu_al);
	emu_flags_zf(emu_al);
	emu_flags_pf(emu_al);
	emu_flags.cf = 0;
	emu_flags.of = 0;
	emu_flags.af = 0;
}


/* AAS */

void emu_aas()
{
	if ((emu_al & 0x0F) > 9 || emu_flags.af) {
		emu_ax -= 0x0106;
		emu_flags.af = 1;
		emu_flags.cf = 1;
		emu_flags.of = (emu_flags.af && emu_al >= 0x80 && emu_al <= 0x85) ? 1 : 0;
		emu_flags.sf = (emu_al >= 0x86 || (emu_flags.af && emu_al <= 0x05)) ? 1 : 0;
	} else {
		emu_flags.af = 0;
		emu_flags.cf = 0;
		emu_flags.of = 0;
		emu_flags.sf = (emu_al >= 0x80);
	}

	emu_flags_zf(emu_al);
	emu_flags_pf(emu_al);

	/* DosBox compatible: do this at the end */
	emu_al &= 0x0F;
}


/* DAA */

void emu_daa()
{
	if ((emu_al & 0x0F) > 0x09 || emu_flags.af) {
		emu_al = emu_al + 0x06;
		emu_flags.af = 1;
	} else {
		emu_flags.af = 0;
	}
	if (emu_al > 0x9F || emu_flags.cf) {
		emu_al = emu_al + 0x60;
		emu_flags.cf = 1;
	} else {
		emu_flags.cf = 0;
	}

	emu_flags_sfb(emu_al);
	emu_flags_zf(emu_al);
	emu_flags_pf(emu_al);
}


/* DAS */

void emu_das()
{
	if ((emu_al & 0x0F) > 0x09 || emu_flags.af) {
		emu_al = emu_al - 0x06;
		emu_flags.af = 1;
	} else {
		emu_flags.af = 0;
	}
	if (emu_al > 0x9F || emu_flags.cf) {
		emu_al = emu_al - 0x60;
		emu_flags.cf = 1;
	} else {
		emu_flags.cf = (emu_al <= 0x05) ? 1 : 0; /* DosBox compatible, according to docs this should just be 0 */
	}

	emu_flags_sfb(emu_al);
	emu_flags_zf(emu_al);
	emu_flags_pf(emu_al);
}
