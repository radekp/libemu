/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_MSC_VER)
	#include <io.h>
	#define unlink _unlink
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
	PACK uint8  type;
	PACK uint16 psp;
	PACK uint16 size;
	PACK uint8  unused[3];
	PACK char   filename[8];
} GCC_PACKED MSB;
MSVC_PACKED_END
assert_compile(sizeof(MSB) == 16);

MSVC_PACKED_BEGIN
typedef struct FCB {
	PACK uint8  driveno;
	PACK char   filename[8];
	PACK char   extension[3];
	PACK uint16 current_block_no;
	PACK uint16 logical_record_size;
	PACK uint32 filesize;
	PACK uint16 last_write_data;
	PACK uint16 last_write_time;
	PACK uint32 reserved1;
	PACK uint32 reserved2;
	PACK uint8  cur_record;
	PACK uint32 random_access_record_number;
} GCC_PACKED FCB;
MSVC_PACKED_END
assert_compile(sizeof(FCB) == 37);

MSVC_PACKED_BEGIN
typedef struct XFCB {
	PACK uint8  ff;
	PACK uint8  xfcb_reserved[5];
	PACK uint8  attribute;
	PACK FCB    fcb;
} GCC_PACKED XFCB;
MSVC_PACKED_END
assert_compile(sizeof(XFCB) == 44);

/* DOS starts its filenumbers at 5, so simulate that by mapping them to system filenumbers */
static FILE *_int21_filemap[20] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static uint16 _int21_DTA_segment = 0x0;
static uint16 _int21_DTA_offset  = 0x0;

void emu_int21_getfile(char *filename, int length, char *buffer)
{
	char tbuf[33];
	char *buf = &tbuf[0];
	char *c;

	strncpy(buf, filename, length);
	buf[length] = '\0';

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
	if (_int21_filemap[0] == NULL) _int21_filemap[0] = stdin;
	if (_int21_filemap[1] == NULL) _int21_filemap[1] = stdout;
	if (_int21_filemap[2] == NULL) _int21_filemap[2] = stderr;

	switch (emu_ah) {
		case 0x06: /* DIRECT CONSOLE I/O - DL -> output char, or 0xFF for input request */
		{          /* Return: AL -> input char, ZF -> input char ready */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:06 ] DIRECT CONSOLE I/O: '%c'\n", emu_dl);
			printf("%c", emu_dl);
		} return;

		case 0x07: /* GET STDIN (ignore CTRL+BREAK) */
		case 0x08: /* GET STDIN */
		{          /* Return: AL -> key */
			extern int emu_int9_key_getascii();
			extern void emu_int9_key_wait();

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:%02X ] GET STDIN\n", emu_ah);

			/* Wait for a key */
			emu_int9_key_wait();

			emu_al = emu_int9_key_getascii();
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
			extern int emu_int9_key_iswaiting();

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:0B ] IS STDIN WAITING\n");
			emu_al = (emu_int9_key_iswaiting() > 0) ? 0xFF : 0x00;
		} return;

		case 0x0C: /* CLEAR KEYBOARD BUFFER AND INVOKE KEYBOARD FUNCTION - AL -> INT21,AL to call after clear */
		{          /* Return: INT21,AL return value */
			extern void emu_int9_key_flush();
			uint8 old_ah = emu_ah;
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:0C ] CLEAR BUFFER AND INVOKE FUNCTION\n");

			emu_int9_key_flush();

			switch (emu_al) {
				case 0x1:
				case 0x6:
				case 0x7:
				case 0x8:
				case 0xA:
					break;
				default:
					fprintf(stderr, "[EMU] Tried to invoke function via INT21:0C which is not acceptable.\n");
					return;
			}

			/* Now call INT21,AL */
			emu_ah = emu_al;
			emu_int21();
			emu_ah = old_ah;
		} return;

		case 0x0E: /* SELECT DISK - DL -> drive number */
		{          /* Return: AL -> number of logic drives */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:0E ] SELECT DISK '%d'\n", emu_dl);
			emu_al = 0x1A;
		} return;

		case 0x0F: /* OPEN FILE USING FCB - DS:DX -> pointer to unopened FSB */
		{          /* Return: AL -> 00 (file opened), FF (file not opened) */
			char filename[13];
			char buf[33];
			char *c;
			FCB *fcb;
			FILE *fp;
			int i;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:0F ] OPEN FILE USING FCB at %X:%X\n", emu_ds, emu_dx);

			fcb = (FCB *)&emu_get_memory8(emu_ds, emu_dx, 0);
			if (fcb->driveno == 0xFF) fcb = &((XFCB *)&emu_get_memory8(emu_ds, emu_dx, 0))->fcb;

			/* Put the pieces of the filename together */
			strncpy(filename, fcb->filename, 8);
			c = strchr(filename, ' ');
			if (c == NULL) c = &filename[8];
			*c = '\0';
			strcat(c, ".");
			strncat(c, fcb->extension, 3);
			c = strchr(filename, ' ');
			if (c == NULL) c = &filename[12];
			*c = '\0';

			emu_int21_getfile(filename, 13, buf);

			fp = fopen(buf, "rb");
			if (fp == NULL) {
				emu_al = 0xFF;
				return;
			}

			/* Find the next open slot for the file number */
			for (i = 5; i < 20; i++) if (_int21_filemap[i] == NULL) break;
			if (i == 20) {
				fprintf(stderr, "[EMU] ERROR: out of file descriptors\n");
				emu_al = 0xFF;
				return;
			}

			_int21_filemap[i] = fp;
			fcb->reserved1 = i;

			/* Get stats for the FCB */
			fseek(fp, 0, SEEK_END);
			fcb->filesize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			fcb->current_block_no = 0;
			fcb->logical_record_size = 128;
			fcb->cur_record = 0;
			/* TODO -- Set create and last-write time */

			emu_al = 0x00;
		} return;

		case 0x10: /* CLOSE FILE USING FCB - DS:DX -> pointer to opened FSB */
		{
			FCB *fcb;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:10 ] CLOSE FILE USING FCB at %X:%X\n", emu_ds, emu_dx);

			fcb = (FCB *)&emu_get_memory8(emu_ds, emu_dx, 0);
			if (fcb->driveno == 0xFF) fcb = &((XFCB *)&emu_get_memory8(emu_ds, emu_dx, 0))->fcb;

			if (fcb->reserved1 < 5 || fcb->reserved1 >= 20 || _int21_filemap[fcb->reserved1] == NULL) {
				emu_al = 0xFF;
				return;
			}

			fclose(_int21_filemap[fcb->reserved1]);
			_int21_filemap[fcb->reserved1] = NULL;
			emu_al = 0x00;
		} return;

		case 0x14: /* SEQUENTIAL READ USING FCB - DS:DX -> pointer to opened FSB */
		{
			FCB *fcb;
			int pos;
			int res;

			if (emu_debug_int) fprintf(stderr, "[EMU] [INT 21:14 ] SEQUANTIAL READ USING FCB at %X:%X\n", emu_ds, emu_dx);

			fcb = (FCB *)&emu_get_memory8(emu_ds, emu_dx, 0);
			if (fcb->driveno == 0xFF) fcb = &((XFCB *)&emu_get_memory8(emu_ds, emu_dx, 0))->fcb;

			if (fcb->reserved1 < 5 || fcb->reserved1 >= 20 || _int21_filemap[fcb->reserved1] == NULL) {
				emu_al = 1; /* END OF FILE, NO DATA */
				return;
			}

			pos = (fcb->current_block_no * 128 + fcb->cur_record) * fcb->logical_record_size;
			fseek(_int21_filemap[fcb->reserved1], pos, SEEK_SET);

			res = fread(&emu_get_memory8(_int21_DTA_segment, _int21_DTA_offset, 0), 1, fcb->logical_record_size, _int21_filemap[fcb->reserved1]);
			fcb->cur_record++;
			if (fcb->cur_record == 128) {
				fcb->current_block_no++;
			}

			if (res == fcb->logical_record_size) {
				emu_al = 0; /* SUCCESS */
			} else if (res == 0) {
				emu_al = 1; /* END OF FILE, NO DATA */
			} else {
				emu_al = 3; /* END OF FILE, PARTIAL DATA */
			}
		} return;

		case 0x19: /* GET CURRENT DEFAULT DRIVE */
		{          /* Return: AL -> current default drive */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:19 ] GET CURRENT DEFAULT DRIVE\n");
			emu_al = 2;
		} return;

		case 0x1A: /* SET DISK TRANSFER ADDRESS - DS:DX -> pointer to DTA */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:1A ] SET DISK TRANSFER ADDRESS to %X:%X\n", emu_ds, emu_dx);

			_int21_DTA_segment = emu_ds;
			_int21_DTA_offset  = emu_dx;
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
			struct tm *tm;
			time_t t;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:2A ] GET DATE\n");
			time(&t);
			tm = localtime(&t);

			emu_al = tm->tm_wday;
			emu_cx = tm->tm_year + 1900;
			emu_dh = tm->tm_mon + 1;
			emu_dl = tm->tm_mday;
		} return;

		case 0x2C: /* GET TIME */
		{          /* Return: CH -> hour, CL -> minutes, DH -> seconds, DL -> msecs */
			struct tm *tm;
			time_t t;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:2C ] GET TIME\n");
			time(&t);
			tm = localtime(&t);

			emu_ch = tm->tm_hour;
			emu_cl = tm->tm_min;
			emu_dh = tm->tm_sec;
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
			emu_int21_getfile((char *)&emu_get_memory8(emu_ds, emu_dx, 0), 32, buf);

			if (emu_cx != 0) fprintf(stderr, "[EMU] [ INT21:3C ] Requesting attributes '%x' for file which is not supported.\n", emu_cx);

			fp = fopen(buf, "wb");
			if (fp == NULL) {
				emu_ax = 0x05; /* ACCESS DENIED */
				emu_flags.cf = 1;
				return;
			}

			/* Find the next open slot for the file number */
			for (i = 5; i < 20; i++) if (_int21_filemap[i] == NULL) break;
			if (i == 20) {
				fprintf(stderr, "[EMU] ERROR: out of file descriptors\n");
				emu_ax = 0x04; /* TOO MANY OPEN FILES */
				emu_flags.cf = 1;
				return;
			}

			_int21_filemap[i] = fp;
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
			emu_int21_getfile((char *)&emu_get_memory8(emu_ds, emu_dx, 0), 32, buf);

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
			for (i = 5; i < 20; i++) if (_int21_filemap[i] == NULL) break;
			if (i == 20) {
				fprintf(stderr, "[EMU] ERROR: out of file descriptors\n");
				emu_ax = 0x04; /* TOO MANY OPEN FILES */
				emu_flags.cf = 1;
				return;
			}

			_int21_filemap[i] = fp;
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

			fclose(_int21_filemap[emu_bx]);
			_int21_filemap[emu_bx] = NULL;
		} return;

		case 0x3F: /* READ FILE - BX -> file handler, CX -> number of bytes, DS:DX -> buffer */
		{          /* Return: AX -> bytes read */
			uint8 *buf;
			int res;

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:3F ] READ FILE '%d' for %d bytes to %04X:%04X\n", emu_bx, emu_cx, emu_ds, emu_dx);
			emu_flags.cf = 0;
			buf = &emu_get_memory8(emu_ds, emu_dx, 0);

			if (emu_bx >= 20 || _int21_filemap[emu_bx] == NULL) {
				emu_ax = 0x06; /* INVALID HANDLE */
				emu_flags.cf = 1;
				return;
			}

			res = fread(buf, 1, emu_cx, _int21_filemap[emu_bx]);

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

			if (emu_bx >= 20 || _int21_filemap[emu_bx] == NULL) {
				emu_ax = 0x06; /* INVALID HANDLE */
				emu_flags.cf = 1;
				return;
			}

			res = fwrite(buf, 1, emu_cx, _int21_filemap[emu_bx]);

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
			emu_int21_getfile((char *)&emu_get_memory8(emu_ds, emu_dx, 0), 32, buf);

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

			res = fseek(_int21_filemap[emu_bx], pos, (emu_al == 0) ? SEEK_SET : ((emu_al == 1) ? SEEK_CUR : SEEK_END));

			if (res < 0) {
				emu_ax = 0x19; /* SEEK ERROR */
				emu_flags.cf = 1;
				return;
			}

			res = ftell(_int21_filemap[emu_bx]);
			emu_ax = res;
			emu_dx = res >> 16;
		} return;

		case 0x43: /* GET/SET FILE ATTRIBUTES - AL -> 0 (get) or 1 (set), DS:DX -> ASCII filename, CX -> attributes to set */
		{          /* Return: CX -> attributes (if get) */
			char buf[33];

			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT21:43 ] GET/SET '%d' FILE ATTRIBUTES for %04X:%04X with '%X'\n", emu_al, emu_ds, emu_dx, emu_cx);
			emu_flags.cf = 0;
			emu_int21_getfile((char *)&emu_get_memory8(emu_ds, emu_dx, 0), 32, buf);

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
