/* $Id$ */

#if !defined(_MSV_VER)
	#define _POSIX_SOURCE
#endif /* _MSV_VER */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_MSC_VER)
	#include <io.h>
	#define close  _close
	#define fileno _fileno
	#define read   _read
	#define write  _write
	#define unlink _unlink
	#define lseek  _lseek
	#define access _access
	#define F_OK   0
	#define new    _new
#else
	#include <unistd.h>
#endif /* _MSC_VER */
#include "types.h"
#include "libemu.h"
#include "bios.h"

MSVC_PACKED_BEGIN
typedef struct MSB {
	uint8 type;
	uint16 psp;
	uint16 size;
	uint8 unused[3];
	char filename[8];
} GCC_PACKED MSB;
MSVC_PACKED_END

/* DOS starts its filenumbers at 5, so simulate that by mapping them to system filenumbers */
static int _int21_filemap[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void emu_int21_getfile(uint16 segment, uint16 offset, char *buffer)
{
	char tbuf[33];
	char *buf = &tbuf[0];
	char *c;

	strncpy(buf, (char *)&emu_get_memory8(segment, offset, 0), 32);
	buf[32] = '\0';

	/* Convert all names to lowercase */
	c = buf;
	while (*c != '\0') {
		if (*c >= 'A' && *c <= 'Z') *c = *c - 'A' + 'a';
		if (*c == '\\') *c = '/';
		c++;
	}
	/* Skip any drive prefixes */
	if (strncmp(buf + 1, ":/", 2) == 0) buf += 3;

	if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21 ] File name is '%s'\n", buf);
	strcpy(buffer, "data/");
	strcat(buffer, buf);
}

void emu_int21()
{
	switch (emu_ah) {
		case 0x06: /* DIRECT CONSOLE I/O - DL -> output char, or 0xFF for input request */
		{          /* Return: AL -> input char, ZF -> input char ready */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:06 ] DIRECT CONSOLE I/O: '%c'\n", emu_dl);
			printf("%c", emu_dl);
		} return;

		case 0x08: /* GET STDIN */
		{          /* Return: AL -> key */
			extern int emu_int9_getasciikey();

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:08 ] GET STDIN\n");
			emu_al = emu_int9_getasciikey();
			if (emu_al == 0x0A) emu_al = 0x0D;
		} return;

		case 0x09: /* PRINT STRING - DS:DX -> string terminated by '$' */
		{          /* Return: */
			char *buf;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:09 ] PRINT STRING at %04X:%04X\n", emu_ds, emu_dx);
			buf = (char *)&emu_get_memory8(emu_ds, emu_dx, 0);
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:09 ] String is '");
			while (*buf != '$') {
				printf("%c", *buf);
				if (emu_debug_int) fprintf(stderr, "%c", *buf);
				buf++;
			}
			if (emu_debug_int) fprintf(stderr, "'\n");
		} return;

		case 0x0B: /* IS STDIN WAITING */
		{          /* Return: AL -> 00 (no key waiting), FF (key waiting) */
			extern int emu_int9_keywaiting();

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:0B ] IS STDIN WAITING\n");
			emu_al = (emu_int9_keywaiting() > 0) ? 0xFF : 0x00;
		} return;

		case 0x0E: /* SELECT DISK - DL -> drive number */
		{          /* Return: AL -> number of logic drives */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:0E ] SELECT DISK '%d'\n", emu_dl);
			emu_al = 0x1A;
		} return;

		case 0x19: /* GET CURRENT DEFAULT DRIVE */
		{          /* Return: AL -> current default drive */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:19 ] GET CURRENT DEFAULT DRIVE\n");
			emu_al = 2;
		} return;

		case 0x1B: /* GET ALLOCATION TABLE INFORMATION */
		{          /* Return AX -> sectors per cluster, CX -> bytes per sector, DX -> clusters on disk, DS:BX -> pointer to MDB */
			emu_ax = 0x7F;   /* 127 sectors per cluster */
			emu_cx = 0x200;  /* 512 bytes per sector */
			emu_dx = 0x4000; /* About 1.2 GiB total diskspace */
			emu_ds = 0x0;    /* TODO -- MDB support */
			emu_bx = 0x0;
		} return;

		case 0x25: /* SET INTERRUPT VECTOR - AL -> interrupt number, DS:DX -> interrupt pointer */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:25 ] SET INTERRUPT VECTOR '0x%X' to %04X:%04X\n", emu_al, emu_ds, emu_dx);
			emu_get_memory16(0, 0, emu_al * 4 + 0) = emu_dx;
			emu_get_memory16(0, 0, emu_al * 4 + 2) = emu_ds;
		} return;

		case 0x29: /* PARSE FILENAME FOR FCB - AL -> control, DS:SI -> filename, ES:DI -> FCB (unopened) */
		{          /* Return: AL -> result, DS:SI -> first char of filename, ES:DI -> FCB (unopened) */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:29 ] PARSE FILENAME FOR FCB at %X with file %04X:%04X to FSB %04X:%04X\n", emu_al, emu_ds, emu_si, emu_es, emu_di);

			/* TODO -- Support this */
			emu_al = 0xFF;
		} return;

		case 0x2A: /* GET DATE */
		{          /* Return: AL -> day of week, CX -> year, DH -> month, DL -> day */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:2A ] GET DATE\n");
			/* TODO -- Give the real date */
			emu_al = 0;
			emu_cx = 2000;
			emu_dh = 1;
			emu_dl = 1;
		} return;

		case 0x2C: /* GET TIME */
		{          /* Return: CH -> hour, CL -> minutes, DH -> seconds, DL -> msecs */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:2C ] GET TIME\n");
			/* TODO -- Give the real time */
			emu_ch = 0;
			emu_cl = 0;
			emu_dh = 0;
			emu_dl = 0;
		} return;

		case 0x30: /* GET DOS VERSION */
		{          /* Return: AL -> major version, AH -> minor version, BH -> FF if OEM key, BL:CX -> OEM key */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:30 ] GET DOS VERSION\n");
			/* We indentify ourself as PC-DOS, no OEM key */
			if (emu_al == 0) emu_bh = 0xFF; /* MS-Dos */
			if (emu_al == 1) emu_bh = 0x10; /* Indentify as in HMA */

			emu_al = 5;
			emu_ah = 0;

			emu_bl = 0;
			emu_cx = 0;
		} return;

		case 0x33: /* GET/SET SYSTEM VALUES - AL -> 00 (get Ctrl-Break), 01, (set Ctrl-Break), 02 (set extended), 05 (get boot drive), DL -> 00 (set off), 01 (set on), boot drive */
		{          /* Return: DL -> 00 (off), 01 (on), boot drive */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:33 ] GET/SET SYSTEM VALUES 0x%X with 0x%X\n", emu_al, emu_dl);

			if (emu_al == 0x0) emu_dl = 0x0;
			if (emu_al == 0x5) emu_dl = 0x2;
		} return;

		case 0x35: /* GET INTERRUPT VECTOR - AL -> interrupt number */
		{          /* Return: ES:BX -> intterupt pointer */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:35 ] GET INTERRUPT VECTOR '0x%X'\n", emu_al);
			emu_bx = emu_get_memory16(0, 0, emu_al * 4 + 0);
			emu_es   = emu_get_memory16(0, 0, emu_al * 4 + 2);
		} return;

		case 0x36: /* GET FREE DISKSPACE, DL -> drive number */
		{          /* Return: AX -> sectors per cluster, BX -> available clusters, CX -> bytes per sector, DX -> clusters per drive */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:36 ] GET FREE DISKSPACE for '0x%X'\n", emu_dl);
			emu_ax = 0x7F;   /* 127 sectors per cluster */
			emu_bx = 0x1000; /* About 300 MiB free diskspace */
			emu_cx = 0x200;  /* 512 bytes per sector */
			emu_dx = 0x4000; /* About 1.2 GiB total diskspace */
		} return;

		case 0x3B: /* CHANGE CURRENT DIRECTORY - DS:DX -> ASCII directory name */
		{          /* Return: */
			char fbuf[1024];
			char *buf = &fbuf[0];
			char *c;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:3B ] CHANGE CURRENT DIRECTORY at %04X:%04X\n", emu_ds, emu_dx);
			emu_flags.cf = 0;
			strncpy(buf, (char *)&emu_get_memory8(emu_ds, emu_dx, 0), 1024);

			/* Convert all names to lowercase */
			c = buf;
			while (*c != '\0') {
				if (*c >= 'A' && *c <= 'Z') *c = *c - 'A' + 'a';
				if (*c == '\\') *c = '/';
				c++;
			}
			if (strncmp(buf, "c:/", 3) == 0) buf += 3;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:3B ] Directory name is '%s'\n", buf);

			/* We don't really change the directory .. we don't really care */
		} return;

		case 0x3C: /* CREATE FILE - DS:DX -> ASCII filename, CX -> attributes for file */
		{          /* Return: AX -> file handler */
			char buf[33];
			FILE *fp;
			int i;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:3C ] CREATE FILE at %04X:%04X with attribute %d\n", emu_ds, emu_dx, emu_cx);
			emu_flags.cf = 0;
			emu_int21_getfile(emu_ds, emu_dx, buf);

			if (emu_cx != 0) fprintf(stderr, "[EMU] [ INT21:3C ] Requesting attributes '%x' for file which is not supported.\n", emu_cx);

			fp = fopen(buf, "wb");
			if (fp == NULL) {
				emu_ax = 0x05; /* ACCESS DENIED */
				emu_flags.cf = 1;
				return;
			}

			/* Find the next open slot for the file number */
			for (i = 5; i < 20; i++) if (_int21_filemap[i] == 0) break;
			if (i == 20) {
				fprintf(stderr, "[EMU] ERROR: out of file descriptors\n");
				emu_ax = 0x04; /* TOO MANY OPEN FILES */
				emu_flags.cf = 1;
				return;
			}

			_int21_filemap[i] = fileno(fp);
			emu_ax = i;
		} return;

		case 0x3D: /* OPEN FILE - DS:DX -> ASCII filename, AL -> access mode */
		{          /* Return: AX -> file handler */
			char mode[4] = {'r', 'b', '\0', '\0'};
			char buf[33];
			FILE *fp;
			int i;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:3D ] OPEN FILE at %04X:%04X with mode %d\n", emu_ds, emu_dx, emu_al);
			emu_flags.cf = 0;
			emu_int21_getfile(emu_ds, emu_dx, buf);

			if ((emu_al & 0x1) == 0x1) mode[0] = 'w';
			if ((emu_al & 0x2) == 0x2) mode[2] = '+';
			if (emu_al > 3) fprintf(stderr, "[EMU] [ INT21:3D ] Requesting mode '%x' which is not completely supported.\n", emu_al);
			emu_ax = 0;

			fp = fopen(buf, mode);
			if (fp == NULL) {
				emu_ax = 0x02; /* FILE NOT FOUND */
				emu_flags.cf = 1;
				return;
			}

			/* Find the next open slot for the file number */
			for (i = 5; i < 20; i++) if (_int21_filemap[i] == 0) break;
			if (i == 20) {
				fprintf(stderr, "[EMU] ERROR: out of file descriptors\n");
				emu_ax = 0x04; /* TOO MANY OPEN FILES */
				emu_flags.cf = 1;
				return;
			}

			_int21_filemap[i] = fileno(fp);
			emu_ax = i;
		} return;

		case 0x3E: /* CLOSE FILE - BX -> file handler */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:3E ] CLOSE FILE '%d'\n", emu_bx);
			emu_flags.cf = 0;

			if (emu_bx < 5 || emu_bx >= 20) {
				emu_ax = 0x06; /* INVALID HANDLE */
				emu_flags.cf = 1;
				return;
			}

			close(_int21_filemap[emu_bx]);
			_int21_filemap[emu_bx] = 0;
		} return;

		case 0x3F: /* READ FILE - BX -> file handler, CX -> number of bytes, DS:DX -> buffer */
		{          /* Return: AX -> bytes read */
			uint8 *buf;
			int res;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:3F ] READ FILE '%d' for %d bytes to %04X:%04X\n", emu_bx, emu_cx, emu_ds, emu_dx);
			emu_flags.cf = 0;
			buf = &emu_get_memory8(emu_ds, emu_dx, 0);

			if (emu_bx < 5 || emu_bx >= 20) {
				emu_ax = 0x06; /* INVALID HANDLE */
				emu_flags.cf = 1;
				return;
			}

			res = read(_int21_filemap[emu_bx], buf, emu_cx);

			if (res < 0) {
				emu_ax = 0x1E; /* READ FAULT */
				emu_flags.cf = 1;
				return;
			}
			emu_ax = res;
		} return;

		case 0x40: /* WRITE FILE - BX -> file handler, CX -> number of bytes, DS:DX -> buffer */
		{          /* Return: AX -> bytes written */
			uint8 *buf;
			int res;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:40 ] WRITE FILE '%d' for %d bytes from %04X:%04X\n", emu_bx, emu_cx, emu_ds, emu_dx);
			emu_flags.cf = 0;
			buf = &emu_get_memory8(emu_ds, emu_dx, 0);

			if (emu_bx < 5 || emu_bx >= 20) {
				emu_ax = 0x06; /* INVALID HANDLE */
				emu_flags.cf = 1;
				return;
			}

			res = write(_int21_filemap[emu_bx], buf, emu_cx);

			if (res < 0) {
				emu_ax = 0x1D; /* WRITE FAULT */
				emu_flags.cf = 1;
				return;
			}
			emu_ax = res;
		} return;

		case 0x41: /* DELETE FILE - DS:DX -> ASCII filename */
		{          /* Return: */
			char buf[33];

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:41 ] DELETE FILE at %04X:%04X\n", emu_ds, emu_dx);
			emu_flags.cf = 0;
			emu_int21_getfile(emu_ds, emu_dx, buf);

			/* Delete the file */
			unlink(buf);
		} return;

		case 0x42: /* MOVE FILE POINTER - AL -> method, BX -> file handler, CX -> high order bytes to move, DX -> low order bytes to move */
		{          /* Return: DX:AX -> new location */
			int pos;
			int res;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:42 ] MOVE FILE '%d' POINTER to %d via method %d\n", emu_bx, (int32)((emu_cx << 16) + emu_dx), emu_al);
			emu_flags.cf = 0;

			pos = (int32)((emu_cx << 16) + emu_dx);

			if (emu_bx < 5 || emu_bx >= 20) {
				emu_ax = 0x06; /* INVALID HANDLE */
				emu_flags.cf = 1;
				return;
			}

			res = lseek(_int21_filemap[emu_bx], pos, (emu_al == 0) ? SEEK_SET : ((emu_al == 1) ? SEEK_CUR : SEEK_END));

			if (res < 0) {
				emu_ax = 0x19; /* SEEK ERROR */
				emu_flags.cf = 1;
				return;
			}
			emu_ax = res;
			emu_dx = res >> 16;
		} return;

		case 0x43: /* GET/SET FILE ATTRIBUTES - AL -> 0 (get) or 1 (set), DS:DX -> ASCII filename, CX -> attributes to set */
		{          /* Return: CX -> attributes (if get) */
			char buf[33];

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:43 ] GET/SET '%d' FILE ATTRIBUTES for %04X:%04X with '%X'\n", emu_al, emu_ds, emu_dx, emu_cx);
			emu_flags.cf = 0;
			emu_int21_getfile(emu_ds, emu_dx, buf);

			if (access(buf, F_OK) != 0) {
				emu_ax = 0x02; /* FILE NOT FOUND */
				emu_flags.cf = 1;
				return;
			}

			if (emu_al != 0 && emu_debug_int) fprintf(stderr, "[EMU] [ INT21:43 ] Ignoring SET\n");

			/* Assume it is always a file */
			if (emu_al == 0) emu_ax = emu_cx = 0x20;
		} return;

		case 0x44: /* I/O Control - AL -> function, BX -> file handler, CX -> number of bytes, DS:DX -> data or buffer */
		{          /* Return: */
			emu_flags.cf = 0;

			switch (emu_al) {
				case 0x00: /* GET DEVICE INFORMATION - BX -> file handler */
				{          /* Return: DX -> device information */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:44:00 ] GET DEVICE INFORMATION on file '%X'\n", emu_bx);
					if (emu_bx <= 4) emu_dx = 0x80D3;
					else emu_dx = 0x02;

					emu_ax = emu_dx;
				} return;

				case 0x01: /* SET DEVICE INFORMATION - BX -> file handler, DX -> device information */
				{          /* Return: DX -> device information */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:44:01 ] SET DEVICE INFORMATION on file '%X' to %d\n", emu_bx, emu_dl);
				} return;

				case 0x0E: /* GET LOGICAL DRIVE - BL -> physical drive number */
				{          /* Return: AL -> 0 */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:44:0E ] GET LOGICAL DRIVE of '%X'\n", emu_bl);
					emu_al = 0;
				} return;

				default:
					fprintf(stderr, "[EMU] [ INT21:44 ] IOCTL '%d' Not Yet Implemented\n", emu_al);
					bios_uninit(1);
			}
		} return;

		case 0x47: /* GET CURRENT DIRECTORY - DL -> drive number, DS:SI -> buffer */
		{          /* Return: DS:SI -> ASCII directory name */
			char *buf;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:47 ] GET CURRENT DIRECTORY\n");
			emu_flags.cf = 0;

			/* Fake we are in the root of the drive, at all times */
			buf = (char *)&emu_get_memory8(emu_ds, emu_si, 0);
			buf[0] = '\0';
			emu_ax = 0x0100;
		} return;

		case 0x48: /* ALLOCATE MEMORY BLOCK - BX -> block size in paragraphs */
		{          /* Return: */
			uint16 address, biggest_size;
			MSB *msb, *env;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:48 ] ALLOCATE MEMORY BLOCK of size %X\n", emu_bx << 4);
			emu_flags.cf = 0;

			/* The first MSB is always at segment 0x68 */
			address = 0x68;
			msb = (MSB *)&emu_get_memory8(address, 0, 0);
			/* Next MSB is always the env */
			env = (MSB *)&emu_get_memory8(address + msb->size + 0x1, 0, 0);

			biggest_size = 0;

			/* Walk all valid MSBs */
			while (1) {
				/* Find the next MSB */
				MSB *next = (MSB *)&emu_get_memory8(address + msb->size + 0x1, 0, 0);

				/* This MSB is in use, go to the next one */
				if (msb->psp != 0x0) {
					if (msb->type == 0x5A) break;
					address += msb->size + 0x1;
					msb = next;
					continue;
				}

				/* The next MSB is also free, merge them */
				if (msb->type != 0x5A && next->psp == 0x0) {
					msb->type = next->type;
					msb->size += next->size + 0x1;
					/* Don't go to the next, as the next MSB might be free too */
					continue;
				}

				/* If we don't fit in this MSB, remember the biggest size we came across we do fit in */
				if (emu_bx > msb->size) {
					if (msb->size > biggest_size) biggest_size = msb->size;
					if (msb->type == 0x5A) break;
					address += msb->size + 0x1;
					msb = next;
					continue;
				}

				/* If we fit perfectly in this MSB, put us in there */
				if (emu_bx == msb->size) {
					msb->psp = env->psp;
					memcpy(msb->filename, env->filename, 8);
					emu_ax = address + 0x1;
					return;
				}

				/* We fit in here, so allocate it */
				break;
			}

			/* See if there was enough memory free */
			if (emu_bx > msb->size) {
				emu_flags.cf = 1;
				emu_ax = 0x08; /* INSUFFICIENT MEMORY */
				emu_bx = biggest_size == 0 ? msb->size : biggest_size;
				return;
			}

			/* We fit in this MSB, so put us there */
			{
				MSB *new = (MSB *)&emu_get_memory8(address + emu_bx + 0x1, 0, 0);
				new->type = msb->type;
				new->psp  = 0x0;
				new->size = msb->size - emu_bx - 0x1;
			}

			/* And update the MSB */
			msb->type = 0x4D;
			msb->psp  = env->psp;
			msb->size = emu_bx;
			memcpy(msb->filename, env->filename, 8);

                        emu_ax = address + 0x1;
		} return;

		case 0x49: /* FREE ALLOCATED MEMORY BLOCK - ES -> segment of block */
		{          /* Return: */
			MSB *cur;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:49 ] FREE MEMORY BLOCK 0x%04X\n", emu_es);
			emu_flags.cf = 0;

			cur = (MSB *)&emu_get_memory8(emu_es - 0x1, 0, 0);

			/* Check if it is a MSB (as far as this is a 'valid' check) */
			if (cur->type != 0x5A && cur->type != 0x4D) {
				emu_flags.cf = 1;
				emu_ax = 0x09; /* INVALID MEMORY BLOCK ADDRESS */
				return;
			}

			/* Free the MSB by simply making the PSP empty */
			cur->psp = 0x0;
		} return;

		case 0x4A: /* MODIFY ALLOCATED MEMORY BLOCK - BX -> block size in paragraphs, ES -> segment of block */
		{          /* Return: ES:BX -> intterupt pointer */
			MSB *cur, *next, *new;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:4A ] MODIFY MEMORY BLOCK 0x%04X to size %X\n", emu_es, emu_bx << 4);
			emu_flags.cf = 0;

			cur  = (MSB *)&emu_get_memory8(emu_es - 0x1, 0, 0);
			next = (MSB *)&emu_get_memory8(emu_es + cur->size, 0, 0);
			new  = (MSB *)&emu_get_memory8(emu_es + emu_bx, 0, 0);

			/* First we 'free' the cur block, so merge it with the next block(s) if it is free */
			while (cur->type != 0x5A && next->psp == 0x0) {
				cur->type = next->type;
				cur->size += next->size + 0x1;
			}

			/* Check if we try to allocate a block bigger than the free memory */
			if (emu_bx > cur->size) {
				emu_flags.cf = 1;
				emu_ax = 0x08; /* INSUFFICIENT MEMORY */
				emu_bx = cur->size;
				return;
			}
			emu_ax = emu_es;

			/* If we allocate the complete size, there won't be a 'new' MSB */
			if (emu_bx != cur->size) {
				/* Assign the values for the new MSB */
				new->type = cur->type;
				new->psp  = 0x0;
				new->size = cur->size - emu_bx - 0x1;

				/* And update the current MSB */
				cur->type = 0x4D;
				cur->size = emu_bx;
			}
		} return;

		case 0x4C: /* QUIT WITH EXIT CODE - AL -> exit code */
		{          /* Return: */
			fprintf(stderr, "[EMU] INT 0x21 AL 0x4C application termination\n");
			bios_uninit(0);
		} return;

		default:
			fprintf(stderr, "[EMU] [ INT21:%02X ] Not Yet Implemented\n", emu_ah);
			bios_uninit(1);
	}
}
