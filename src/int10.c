#include <SDL.h>
#ifndef WIN32
#include <ncurses.h>
#ifdef UNICODE
#include <locale.h>
#include <langinfo.h>
#endif /* UNICODE */
#endif /* WIN32 */
#include "types.h"
#include "libemu.h"
#include "bios.h"
#include "pic.h"
#include "int10.h"

static SDL_Surface *_gfx_surface = NULL;
static uint8 *_gfx_screen = NULL;
static int _gfx_mode   = 0;
static int _gfx_height = 0;
static int _gfx_width  = 0;
static int _gfx_pal    = 0;
static int _gfx_high   = 0;
static int _gfx_type   = 0;
static int _gfx_palette_update = 0;
static int _gfx_lock   = 0;

static uint8 _colours_cga_0x04[2][2][4] = {
	{ { 0, 20, 160, 168 }, { 0, 93, 233, 253 }, },
	{ { 0, 22, 162, 182 }, { 0, 95, 235, 255 }, },
};

#ifndef WIN32
static int _colours_text_0x03[] = {
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_YELLOW,
	COLOR_WHITE
};

#ifdef UNICODE
static unsigned char _is_utf8 = false;

static char *_ncurses_acs_map_utf8[256] = {
	"\u2007", "\u263A", "\u263B", "\u2665", "\u2666", "\u2663", "\u2660", "\u2022", /* 000 - 007 */
	"\u25D8", "\u25CB", "\u25D9", "\u2642", "\u2640", "\u366A", "\u266B", "\u263C", /* 008 - 015 */
	"\u25BA", "\u25C4", "\u2195", "\u203C", "\u00B6", "\u00A7", "\u25AC", "\u21A8", /* 016 - 023 */
	"\u2191", "\u2193", "\u2192", "\u2190", "\u221F", "\u2194", "\u25B2", "\u25BC", /* 024 - 031 */
	  "\x20",   "\x21",   "\x22",   "\x23",   "\x24",   "\x25",   "\x26",   "\x27", /* 032 - 039 */
	  "\x28",   "\x29",   "\x2A",   "\x2B",   "\x2C",   "\x2D",   "\x2E",   "\x2F", /* 040 - 047 */
	  "\x30",   "\x31",   "\x32",   "\x33",   "\x34",   "\x35",   "\x36",   "\x37", /* 048 - 055 */
	  "\x38",   "\x39",   "\x3A",   "\x3B",   "\x3C",   "\x3D",   "\x3E",   "\x3F", /* 056 - 063 */
	  "\x40",   "\x41",   "\x42",   "\x43",   "\x44",   "\x45",   "\x46",   "\x47", /* 064 - 071 */
	  "\x48",   "\x49",   "\x4A",   "\x4B",   "\x4C",   "\x4D",   "\x4E",   "\x4F", /* 072 - 079 */
	  "\x50",   "\x51",   "\x52",   "\x53",   "\x54",   "\x55",   "\x56",   "\x57", /* 080 - 087 */
	  "\x58",   "\x59",   "\x5A",   "\x5B",   "\x5C",   "\x5D",   "\x5E",   "\x5F", /* 088 - 095 */
	  "\x60",   "\x61",   "\x62",   "\x63",   "\x64",   "\x65",   "\x66",   "\x67", /* 096 - 103 */
	  "\x68",   "\x69",   "\x6A",   "\x6B",   "\x6C",   "\x6D",   "\x6E",   "\x6F", /* 104 - 111 */
	  "\x70",   "\x71",   "\x72",   "\x73",   "\x74",   "\x75",   "\x76",   "\x77", /* 112 - 119 */
	  "\x78",   "\x79",   "\x7A",   "\x7B",   "\x7C",   "\x7D",   "\x7E", "\u2302", /* 120 - 127 */
	"\u00C7", "\u00FC", "\u00E9", "\u00E2", "\u00E4", "\u00E0", "\u00E5", "\u00E7", /* 128 - 135 */
	"\u00EA", "\u00EB", "\u00E9", "\u00EF", "\u00EE", "\u00EC", "\u00C4", "\u00C5", /* 136 - 143 */
	"\u00C9", "\u00E6", "\u00C6", "\u00F4", "\u00F6", "\u00F2", "\u00FB", "\u00F9", /* 144 - 151 */
	"\u00FF", "\u00D6", "\u00DC", "\u00A2", "\u00A3", "\u00A5", "\u20A7", "\u0192", /* 152 - 159 */
	"\u00E1", "\u00ED", "\u00F3", "\u00FA", "\u00F1", "\u00D1", "\u00AA", "\u00BA", /* 160 - 167 */
	"\u00BF", "\u2310", "\u00AC", "\u00BD", "\u00BC", "\u00A1", "\u00AB", "\u00BB", /* 168 - 175 */
	"\u2591", "\u2592", "\u2593", "\u2502", "\u2524", "\u2561", "\u2562", "\u2556", /* 176 - 183 */
	"\u2555", "\u2563", "\u2551", "\u2557", "\u255D", "\u255C", "\u255B", "\u2510", /* 184 - 191 */
	"\u2514", "\u2534", "\u252C", "\u251C", "\u2500", "\u253C", "\u255E", "\u255F", /* 192 - 199 */
	"\u255A", "\u2554", "\u2569", "\u2566", "\u2560", "\u2550", "\u256C", "\u2567", /* 200 - 207 */
	"\u2568", "\u2564", "\u2565", "\u2559", "\u2558", "\u2552", "\u2553", "\u256B", /* 208 - 215 */
	"\u256A", "\u2518", "\u250C", "\u2588", "\u2584", "\u258C", "\u2590", "\u2580", /* 216 - 223 */
	"\u03B1", "\u00DF", "\u0393", "\u03C0", "\u03A3", "\u03C3", "\u00B5", "\u03C4", /* 224 - 231 */
	"\u03A6", "\u0398", "\u03A9", "\u03B4", "\u221E", "\u03C6", "\u03B5", "\u2229", /* 232 - 239 */
	"\u2261", "\u00B1", "\u2265", "\u2264", "\u2320", "\u2321", "\u00F7", "\u2248", /* 240 - 247 */
	"\u00B0", "\u2219", "\u00B7", "\u221A", "\u207F", "\u00B2", "\u25A0", "\u00A0", /* 248 - 255 */
};
#endif /* UNICODE */

static unsigned char _ncurses_acs_map[128] = {
	255, 255, 255, 255, 255, 255, 255, 255, /* 128 - 135 */
	255, 255, 255, 255, 255, 255, 255, 255, /* 136 - 143 */
	255, 255, 255, 255, 255, 255, 255, 255, /* 144 - 151 */
	255, 255, 255, 255, '}', 165, 255, 255, /* 152 - 159 */
	255, 255, 255, 255, 255, 255, 255, '~', /* 160 - 167 */
	255, 's', 172, 255, 255, 255, 255, 255, /* 168 - 175 */
	255, 255, 'a', 'x', 'u', 255, 255, 255, /* 176 - 183 */
	255, 255, 255, 255, 255, 255, 255, 'k', /* 184 - 191 */
	'm', 'v', 'w', 't', 'q', 'n', 255, 255, /* 192 - 199 */
	255, 255, 255, 255, 255, 255, 255, 255, /* 200 - 207 */
	255, 255, 255, 255, 255, 255, 255, 255, /* 208 - 215 */
	255, 'j', 'l', 255, 255, 255, 255, 255, /* 216 - 223 */
	255, 255, 255, '{', 255, 255, 255, 255, /* 224 - 231 */
	255, 255, 255, 255, 255, 255, 255, 255, /* 232 - 239 */
	255, 'g', 'z', 'y', 255, 255, 255, 255, /* 240 - 247 */
	'f', 255, 255, 255, 255, 255, 255, 255, /* 248 - 255 */
};

static uint8 _ncurses_keymap[] = {
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x1C,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	0x0B, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,    0,    0,    0,    0,    0,    0,
	   0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C,    0,    0,    0,    0,    0,
	   0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0, 0x50, 0x48, 0x4B, 0x4D,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};

#endif /* WIN32 */

static uint8 _SDL_keymap[] = {
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x1C,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x01,    0,    0,    0,    0,
	0x39,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	0x0B, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,    0,    0,    0,    0,    0,    0,
	   0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C,    0,    0,    0,    0,    0,
	   0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
	   0, 0x4F, 0x50, 0x51, 0x4B, 0x1C, 0x4D, 0x47, 0x48, 0x49,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};

enum { INT9_KEYBUF_SIZE = 32 };
static uint8 _int9_key[INT9_KEYBUF_SIZE];
static uint8 _int9_pos = 0;
static uint8 _int9_used = 0;
static int _int9_lastkey = 0;

int emu_int9_keywaiting()
{
	return _int9_used;
}

uint8 emu_io_read_060()
{
	if (_int9_used == 0) return 0xFF;

	uint8 key = _int9_key[_int9_pos];
	if (++_int9_pos >= INT9_KEYBUF_SIZE) _int9_pos -= INT9_KEYBUF_SIZE;
	_int9_used--;

	return key;
}

int emu_int9_getasciikey()
{
	emu_io_read_060();
	emu_io_read_060();
	return _int9_lastkey;
}

static void _int9_keyadd(uint8 key)
{
	uint8 start = _int9_pos + _int9_used;
	if (start >= INT9_KEYBUF_SIZE) start -= INT9_KEYBUF_SIZE;
	_int9_key[start] = key;
	_int9_used++;

	if (emu_flags.inf) emu_hard_int(0x9);
	if (emu_flags.inf) emu_hard_int(0x9);
}

uint8 emu_io_read_3D9()
{
	return 0x08; // TODO -- Find the correct value
}
void emu_io_write_3D9(uint8 value)
{
	// value & 0x7 - Screen RGB
	_gfx_high = (value >> 3) & 1;
	// (value >> 4) - Screen high
	_gfx_pal = (value >> 5) & 1;
}
uint8 emu_io_read_3DA()
{
	static uint8 flip = 0;

	flip++;
	if (flip == 20) flip = 0;

	uint8 value = 0;
	if (flip > 10) value |= 0x01;
	if (flip > 18) value |= 0x08;

	return value;
}

static void _int10_palette(uint8 type, uint8 value)
{
	if (type == 0) return; // Background colour
	_gfx_pal = value;
}

void emu_int10_uninit(uint8 wait)
{
	pic_timer_del(emu_int10_update);

	_gfx_lock = 1;
	if (_gfx_type == 1) {
#ifndef WIN32
		if (wait) {
			attr_set(0, 7, NULL);
			mvaddstr(0, 0, "Press any key to exit ... ");
			refresh();

			while (getch() != -1) {}
			timeout(-1);
			getch();
		}
		endwin();
#endif /* WIN32 */
	}
	if (_gfx_type == 2) {
		SDL_Quit();
	}

	_gfx_type = 0;
	_gfx_lock = 0;
}

enum {
	VGA_DAC_NOT_INITIALIZED = 0,
	VGA_DAC_READ,
	VGA_DAC_WRITE,
};

typedef struct VGADac {
	uint8 state;
	uint8 colour; // 0 = red, 1 = green, 2 = blue
	uint8 read_index;
	uint8 write_index;

	SDL_Color rgb[256];
} VGADac;

static VGADac *emu_vga_dac;
static void emu_int10_vga_dac_init()
{
	emu_vga_dac = (VGADac *)&emu_get_memory8(VGADAC_MEMORY_PAGE, 0, 0);

	if (emu_vga_dac[0].state == VGA_DAC_NOT_INITIALIZED) {
		emu_vga_dac[0].state  = VGA_DAC_READ;
		emu_vga_dac[0].colour = 0;
		emu_vga_dac[0].read_index  = 0;
		emu_vga_dac[0].write_index = 0;

		int i;
		for (i = 0; i < 256; i++) {
			emu_vga_dac[0].rgb[i].r = ((i >> 5) & 0x7) * 255 / 7;
			emu_vga_dac[0].rgb[i].g = ((i >> 2) & 0x7) * 255 / 7;
			emu_vga_dac[0].rgb[i].b = ((i >> 0) & 0x3) * 255 / 3;
		}
	}
	_gfx_palette_update = 1;
}
uint8 emu_io_read_3C7()
{
	return emu_vga_dac[0].state == VGA_DAC_WRITE ? 0x3 : 0x0;
}
void emu_io_write_3C7(uint8 value)
{
	emu_vga_dac[0].state  = VGA_DAC_READ;
	emu_vga_dac[0].colour = 0;
	emu_vga_dac[0].read_index = value;
}

void emu_io_write_3C8(uint8 value)
{
	emu_vga_dac[0].state  = VGA_DAC_WRITE;
	emu_vga_dac[0].colour = 0;
	emu_vga_dac[0].write_index = value;
}

uint8 emu_io_read_3C9()
{
	uint8 value = 0;
	switch (emu_vga_dac[0].colour) {
		case 0: value = emu_vga_dac[0].rgb[emu_vga_dac[0].read_index].r / 4; emu_vga_dac[0].colour = 1; break;
		case 1: value = emu_vga_dac[0].rgb[emu_vga_dac[0].read_index].g / 4; emu_vga_dac[0].colour = 2; break;
		case 2: value = emu_vga_dac[0].rgb[emu_vga_dac[0].read_index].b / 4; emu_vga_dac[0].colour = 0; break;
	}
	if (emu_vga_dac[0].colour == 0) emu_vga_dac[0].read_index++;

	return value;
}
void emu_io_write_3C9(uint8 value)
{
	switch (emu_vga_dac[0].colour) {
		case 0: emu_vga_dac[0].rgb[emu_vga_dac[0].write_index].r = (value & 0x3F) * 4; emu_vga_dac[0].colour = 1; break;
		case 1: emu_vga_dac[0].rgb[emu_vga_dac[0].write_index].g = (value & 0x3F) * 4; emu_vga_dac[0].colour = 2; break;
		case 2: emu_vga_dac[0].rgb[emu_vga_dac[0].write_index].b = (value & 0x3F) * 4; emu_vga_dac[0].colour = 0; break;
	}

	if (emu_vga_dac[0].colour == 0) {
		_gfx_palette_update = 1;
		emu_vga_dac[0].write_index++;
	}
}

void emu_int10_gfx(int mode)
{
	emu_int10_uninit(0);
	_gfx_lock = 1;

#ifdef UNICODE
	setlocale(LC_CTYPE, "");
	_is_utf8 = (strcmp(nl_langinfo(CODESET), "UTF-8") == 0);
#endif /* UNICODE */

	switch (mode) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			_gfx_width  = 80;
			_gfx_height = 25;
			_gfx_pal    = 0;
			_gfx_high   = 0;
			_gfx_type   = 1;
			break;

		case 0x04:
			_gfx_width  = 640;
			_gfx_height = 400;
			_gfx_pal    = 0;
			_gfx_high   = 0;
			_gfx_type   = 2;
			break;

		case 0x0D:
			_gfx_width  = 640;
			_gfx_height = 400;
			_gfx_pal    = 0;
			_gfx_high   = 0;
			_gfx_type   = 2;
			break;

		case 0x12:
			_gfx_width  = 640;
			_gfx_height = 480;
			_gfx_pal    = 0;
			_gfx_high   = 0;
			_gfx_type   = 2;
			break;

		case 0x13:
			_gfx_width  = 640;
			_gfx_height = 400;
			_gfx_pal    = 0;
			_gfx_high   = 0;
			_gfx_type   = 2;
			break;

		default:
			fprintf(stderr, "[EMU] [ INT10:00 ] Video Mode Not Yet Implemented\n");
			bios_uninit(1);
	}
	emu_get_memory8(BIOS_MEMORY_PAGE, 0, BIOS_VIDEO_MODE) = mode;
	_gfx_mode = mode;

	/* Always make sure the VGA DAC is initialized .. allows us to skip
	 *  a good amount of checks ;) */
	emu_int10_vga_dac_init();

	if (_gfx_type == 1) {
#ifndef WIN32
		initscr();
		raw();
		noecho();
		keypad(stdscr, TRUE);
		curs_set(0);
		timeout(0);

		start_color();
		int b, f;
		for (b = 0; b < 8; b++)
			for (f = 0; f < 8; f++)
				init_pair(f + b * 8, _colours_text_0x03[f], _colours_text_0x03[b]);
#endif /* WIN32 */
	}
	if (_gfx_type == 2) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			fprintf(stderr, "[EMU] [ INT10:00 ] Could not initialize SDL: %s\n", SDL_GetError());
			bios_uninit(1);
		}

		SDL_WM_SetCaption("16bit to C Emulator", "");
		_gfx_surface = SDL_SetVideoMode(_gfx_width, _gfx_height, 8, SDL_SWSURFACE | SDL_HWPALETTE);
		if (_gfx_surface == NULL) {
			fprintf(stderr, "[EMU] [ INT10:00 ] Could not set resolution: %s\n", SDL_GetError());
			bios_uninit(1);
		}

		_gfx_screen = (uint8 *)_gfx_surface->pixels;
		memset(_gfx_screen, 0, _gfx_width * _gfx_height);
	}

	_gfx_lock = 0;
	pic_timer_add(emu_int10_update, 0);
}

void emu_int10_update()
{
	if (_gfx_lock) return;

	/* Check if we are int he right mode; if not, switch mode */
	if (emu_get_memory8(BIOS_MEMORY_PAGE, 0, BIOS_VIDEO_MODE) != _gfx_mode) {
		emu_int10_gfx(emu_get_memory8(BIOS_MEMORY_PAGE, 0, BIOS_VIDEO_MODE));
		return;
	}

	if (_gfx_type == 0) return;

	/* Flush output */
	fflush(stdout);
	fflush(stderr);

	int x, y;

	switch (_gfx_mode) {
		case 0x03: /* 80x25 16 color text */
			for (y = 0; y < 25; y++) {
				for (x = 0; x < 160; x += 2) {
#ifndef WIN32
					uint8 chr  = emu_get_memory8(0xB800, 0, y * 160 + x + 0);
					int colour = emu_get_memory8(0xB800, 0, y * 160 + x + 1);
					int fc     = (colour & 0x07);
					int bc     = (colour & 0x70) >> 4;
					int attr   = 0;

					if (colour == 0 || colour == (8 << 4)) chr = ' ';
					if (colour & 0x08) attr |= (fc == 0 && bc == 0) ? A_DIM : A_BOLD;
					if (colour & 0x80) attr |= A_BLINK;

					attr_set(attr, fc + bc * 8, NULL);
#ifdef UNICODE
					if (_is_utf8) {
						mvaddstr(y, x / 2, _ncurses_acs_map_utf8[chr]);
					} else
#endif /* UNICODE */
					if (chr > 0x7F && _ncurses_acs_map[chr & 0x7F] != 0xFF) {
						attr_set(attr | A_ALTCHARSET, fc + bc * 8, NULL);
						mvaddch(y, x / 2, _ncurses_acs_map[chr & 0x7F]);
					} else {
						mvaddch(y, x / 2, chr);
					}
#endif /* WIN32 */
				}
			}
			break;

		case 0x04: /* CGA 320x200x2 (double sized) */
			for (y = 0; y < 200; y++) {
				for (x = 0; x < 320; x += 4) {
					uint16 offset = ((y >> 1) * 320 + x) >> 2;
					if (y & 0x1) offset += 0x2000;
					uint8 data = emu_get_memory8(0xB800, 0, offset);

					int ly = y * 2;
					int lx = x * 2;
					_gfx_screen[lx + ly * _gfx_width + 0] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 6) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 1] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 6) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 2] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 4) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 3] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 4) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 4] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 2) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 5] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 2) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 6] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 0) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 7] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 0) & 0x3)];

					++ly;
					_gfx_screen[lx + ly * _gfx_width + 0] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 6) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 1] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 6) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 2] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 4) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 3] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 4) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 4] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 2) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 5] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 2) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 6] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 0) & 0x3)];
					_gfx_screen[lx + ly * _gfx_width + 7] = _colours_cga_0x04[_gfx_pal][_gfx_high][((data >> 0) & 0x3)];
				}
			}
			break;

		case 0x0D: /* EGA 320x200x16 (double sized) */
		{
			uint8 *data = &emu_get_memory8(0xA000, 0, 0);
			uint8 *gfx  = _gfx_screen;
			for (y = 0; y < 200; y++) {
				for (x = 0; x < 320; x += 8) {
					*gfx++ = (*data >> 7) & 0x01;
					*gfx++ = (*data >> 7) & 0x01;
					*gfx++ = (*data >> 6) & 0x01;
					*gfx++ = (*data >> 6) & 0x01;
					*gfx++ = (*data >> 5) & 0x01;
					*gfx++ = (*data >> 5) & 0x01;
					*gfx++ = (*data >> 4) & 0x01;
					*gfx++ = (*data >> 4) & 0x01;
					*gfx++ = (*data >> 3) & 0x01;
					*gfx++ = (*data >> 3) & 0x01;
					*gfx++ = (*data >> 2) & 0x01;
					*gfx++ = (*data >> 2) & 0x01;
					*gfx++ = (*data >> 1) & 0x01;
					*gfx++ = (*data >> 1) & 0x01;
					*gfx++ = (*data >> 0) & 0x01;
					*gfx++ = (*data >> 0) & 0x01;

					gfx += 640 - 16;
					*gfx++ = (*data >> 7) & 0x01;
					*gfx++ = (*data >> 7) & 0x01;
					*gfx++ = (*data >> 6) & 0x01;
					*gfx++ = (*data >> 6) & 0x01;
					*gfx++ = (*data >> 5) & 0x01;
					*gfx++ = (*data >> 5) & 0x01;
					*gfx++ = (*data >> 4) & 0x01;
					*gfx++ = (*data >> 4) & 0x01;
					*gfx++ = (*data >> 3) & 0x01;
					*gfx++ = (*data >> 3) & 0x01;
					*gfx++ = (*data >> 2) & 0x01;
					*gfx++ = (*data >> 2) & 0x01;
					*gfx++ = (*data >> 1) & 0x01;
					*gfx++ = (*data >> 1) & 0x01;
					*gfx++ = (*data >> 0) & 0x01;
					*gfx++ = (*data >> 0) & 0x01;
					gfx -= 640;
					data++;
				}
				gfx += 640;
			}
		} break;

		case 0x12: /* VGA 640x480x16 */
		{
			uint8 *data = &emu_get_memory8(0xA000, 0, 0);
			uint8 *gfx  = _gfx_screen;
			for (y = 0; y < 480; y++) {
				for (x = 0; x < 640; x += 2) {
					*gfx++ = (*data) & 0x0F;
					*gfx++ = (*data) >> 4;
					data++;
				}
			}
		} break;

		case 0x13: /* VGA 320x200x256 (double sized) */
		{
			uint8 *data = &emu_get_memory8(0xA000, 0, 0);
			uint8 *gfx  = _gfx_screen;
			for (y = 0; y < 400; y += 2) {
				for (x = 0; x < 640; x += 2) {
					*(gfx + 640) = *data;
					*gfx++       = *data;
					*(gfx + 640) = *data;
					*gfx++       = *data;
					data++;
				}
				gfx += 640;
			}
		} break;
	}

	if (_gfx_type == 1) {
#ifndef WIN32
		int i;
		while ((i = getch()) != -1) {
			if (i == 0x03) {
				fprintf(stderr, "[EMU] CTRL+C pressed\n");
				bios_uninit(1);
			}
			if (i >= (int)sizeof(_ncurses_keymap)) continue;
			if (_ncurses_keymap[i] == 0) {
				fprintf(stderr, "[EMU] ERROR: unhandled key %X\n", i);
				continue;
			}
			_int9_keyadd(_ncurses_keymap[i]);
			_int9_keyadd(_ncurses_keymap[i] | 0x80);
			_int9_lastkey = i;
		}
		refresh();
#endif /* WIN32 */
	}
	if (_gfx_type == 2) {
		if (_gfx_palette_update) {
			_gfx_palette_update = 0;
			pic_suspend();
			SDL_SetPalette(_gfx_surface, SDL_LOGPAL | SDL_PHYSPAL, emu_vga_dac[0].rgb, 0, 256);
			pic_resume();
		}

		extern void emu_mouse_change_position(uint16 x, uint16 y);
		extern void emu_mouse_change_button(uint8 left, uint8 press);
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			uint8 keyup = 1;
			switch (event.type) {
				case SDL_QUIT: {
					fprintf(stderr, "[EMU] SDL window closed\n");
					bios_uninit(0);
				} break;

				case SDL_MOUSEMOTION:
					emu_mouse_change_position(event.motion.x, event.motion.y / 2);
					break;

				case SDL_MOUSEBUTTONDOWN:
					emu_mouse_change_button((event.button.button == SDL_BUTTON_LEFT) ? 1 : 0, 1);
					break;
				case SDL_MOUSEBUTTONUP:
					emu_mouse_change_button((event.button.button == SDL_BUTTON_LEFT) ? 1 : 0, 0);
					break;

				case SDL_KEYDOWN:
					keyup = 0;
					/* Fall Through */
				case SDL_KEYUP:
				{
					if (_int9_lastkey == 0x63 && event.key.keysym.sym == 0x132) {
						fprintf(stderr, "[EMU] CTRL+C pressed\n");
						bios_uninit(1);
					}
					if (event.key.keysym.sym >= sizeof(_SDL_keymap)) continue;
					if (_SDL_keymap[event.key.keysym.sym] == 0) {
						fprintf(stderr, "[EMU] ERROR: unhandled key %X\n", event.key.keysym.sym);
						continue;
					}
					if (event.key.keysym.sym > 0xFF) _int9_keyadd(0xE0);
					_int9_keyadd(_SDL_keymap[event.key.keysym.sym] | (keyup ? 0x80 : 0x0));
					_int9_lastkey = event.key.keysym.sym;
				} break;
			}
		}

		SDL_UpdateRect(_gfx_surface, 0, 0, 0, 0);
	}
}

void emu_int10()
{
	switch (emu_ax.h) {
		case 0x00: /* SET VIDEO MODE - AL -> new mode */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:00 ] SET VIDEO MODE '0x%X'\n", emu_ax.l);
			/* Next screen update it will switch to this mode */
			emu_get_memory8(BIOS_MEMORY_PAGE, 0, BIOS_VIDEO_MODE) = emu_ax.l;
		} return;
		case 0x01: /* SET CURSOR TYPE - CH -> cursor starting scan line, CL -> cursor ending scan line */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:01 ] SET CURSOR TYPE '0x%X/0x%X'\n", emu_cx.h, emu_cx.l);
		} return;

		case 0x02: /* SET CURSOR POSITION - BH -> page number, DH -> row, DL -> column */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:02 ] SET CURSOR POSITION '%dx%d' in page 0x%X\n", emu_dx.h, emu_dx.l, emu_bx.h);
		} return;

		case 0x03: /* GET CURSOR POSITION - BH -> page number */
		{          /* Return: CH -> cursor starting scan line, CL -> cursor ending scan line, DH -> row, DL -> column */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:03 ] GET CURSOR POSITION in page 0x%X\n", emu_bx.h);

			emu_cx.h = 0x0;
			emu_cx.l = 0x0;
			emu_dx.h = 0x0;
			emu_dx.l = 0x0;
		} return;

		case 0x05: /* SELECT ACTIVE DISPLAY PAGE - AL -> new page number */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:05 ] SELECT ACTIVE DISPLAY PAGE to 0x%X\n", emu_ax.l);
			/* TODO -- Implement this */
		} return;

		case 0x09: /* WRITE CHAR AND ATTR - AL -> char to write, BH -> display page, BL -> char attribute, CX -> count */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:09 ] WRITE CHAR '%c' AND ATTR '%d' for %d bytes at page 0x%X\n", emu_ax.l, emu_bx.l, emu_bx.h, emu_cx.x);
		} return;

		case 0x0B: /* SET COLOR PALETTE - BH -> type, BL -> value */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:0B ] SET COLOR PALETTE '0x%X/0x%X'\n", emu_bx.l, emu_bx.h);
			_int10_palette(emu_bx.h, emu_bx.l);
		} return;

		case 0x0E: /* WRITE TEXT - AL -> char to write, BH -> display page, BL -> foreground color */
		{          /* Return: */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:0E ] WRITE CHAR '%c' with color %d at page 0x%X\n", emu_ax.l, emu_bx.l, emu_bx.h);
			printf("%c", emu_ax.l);
		} return;

		case 0x0F: /* GET VIDEO STATUS */
		{          /* Return: AH -> number of screen columns, AL -> video mode, BH -> current display page */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:0F ] GET VIDEO STATUS\n");
			emu_ax.h = 80;
			emu_ax.l = emu_get_memory8(BIOS_MEMORY_PAGE, 0, BIOS_VIDEO_MODE);
			emu_bx.h = 0;
		} return;

		case 0x10: /* GET/SET PALETTE */
		{
			switch (emu_ax.l) {
				case 0x12: /* SET DAC BLOCK - BX -> first colour, CX -> number of colours, ES:DX -> table */
				{          /* Return: */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:10:12 ] SET DAC BLOCK from %d to %d at %X:%X\n", emu_bx.x, emu_bx.x + emu_cx.x - 1, emu_es, emu_dx.x);

					int i;
					uint8 *memory = &emu_get_memory8(emu_es, 0, emu_dx.x);
					for (i = emu_bx.x; i < emu_bx.x + emu_cx.x; i++) {
						emu_vga_dac[0].rgb[i].r = ((*memory++) & 0x3F) * 4;
						emu_vga_dac[0].rgb[i].g = ((*memory++) & 0x3F) * 4;
						emu_vga_dac[0].rgb[i].b = ((*memory++) & 0x3F) * 4;
					}
					_gfx_palette_update = 1;
				} return;

				default:
					fprintf(stderr, "[EMU] [ INT10:10:%02X ] Not Yet Implemented\n", emu_ax.l);
					bios_uninit(1);
			}
		} return;

		case 0x11: /* CHARACTER GENERATOR ROUTINE - AL -> type */
		{          /* Return: CX -> byte per character, DL -> rows - 1, ES:BP -> pointer to table */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:11:%02X ] CHARACTER GENERATOR ROUTINE\n", emu_ax.l);
			/* TODO -- Implement this */
		}

		case 0x12: /* VIDEO SUBSYSTEM CONFIGURATION */
		{
			switch (emu_bx.l) {
				case 0x00: /* no idea, used by 'checkit' */
				{          /* Return: */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:12:00 ] no idea\n");
				} return;

				case 0x10: /* VIDEO CONFIGURATION */
				{          /* Return: BH -> 1 (colour) or 0 (mono), BL -> 0 (64k), 1 (128k), 2 (192k), 3 (256k), CH -> feature bits, CL -> switch settings */
					if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:12:10 ] GET VIDEO CONFIGURATION\n");
					emu_bx.h = 0;
					emu_bx.l = 3;
					emu_cx.h = 0;
					emu_cx.l = 0x09; // TODO -- Where does this number come from? Besides DosBox source code
				} return;

				default:
					fprintf(stderr, "[EMU] [ INT10:12:%02X ] Not Yet Implemented\n", emu_bx.l);
					bios_uninit(1);
			}
		} return;

		case 0x1A: /* VIDEO DISPLAY COMBINATION - AL -> 0 (get) or 1 (set), BL -> active display, BH -> inactive display */
		{          /* Return: AL -> 0x1A, BL -> active display, BH -> inactive display */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:1A ] VIDEO DISPLAY COMBINATION\n");
			if (emu_ax.l == 0) {
				emu_bx.x = 0x08; // TODO -- Where does this number come from? Besides DosBox source code
				emu_ax.l = 0x1A;
			} else {
				fprintf(stderr, "[EMU] [ INT10:1A:SET ] Not Yet Implemented\n");
				bios_uninit(1);
			}
		} return;

		case 0x1B: /* VIDEO BIOS STATE INFORMATION - BX -> zero, ES:DI -> pointer to 64byte buffer */
		{          /* Return: AL -> 1B, ES:DI -> pointer to updated buffer */
			if (emu_debug_int) fprintf(stderr, "[EMU] [ INT10:1B ] BIOS STATE INFORMATION (%d) at %04X:%04X\n", emu_bx.x, emu_es, emu_di);

			/* TODO -- Implement this */
			emu_ax.l = emu_ax.h;
		} return;

		/* Used by 'checkit', Hercurlus thing. Ignore. */
		case 0xEF: return;

		/* Used by 'checkit', DESQ/Top thing. Ignore. */
		case 0xFE: return;

		default:
			fprintf(stderr, "[EMU] [ INT10:%02X ] Not Yet Implemented\n", emu_ax.h);
			bios_uninit(1);
	}
};
