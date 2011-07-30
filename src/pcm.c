/* $Id$ */

#include <SDL.h>
#include "types.h"
#include "libemu.h"
#include "pcm.h"
#include "pic.h"

void pcm_init()
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) emu_pcm = 0;
}

typedef struct DMAData {
	uint8  page;
	uint16 offset;
	uint16 count;
} DMAData;

static uint8 _read_status  = 0x00;
static uint8 _read_data    = 0x00;
static uint8 _write_status = 0x00;
static DMAData _dma;
static bool _silence = false;

uint8 emu_io_read_22A()
{
	_read_status &= ~0x80;
	return _read_data;
}

uint8 emu_io_read_22E()
{
	return _read_status;
}

uint8 emu_io_read_22C()
{
	return _write_status;
}

void pcm_sdl_callback(void *userdata, Uint8 *stream, int len)
{
	if (_dma.count == 0xFFFF) return;

	if (len <= _dma.count + 1) {
		if (_silence) {
			memset(stream, 0, len);
		} else {
			memcpy(stream, &emu_get_memory8(_dma.page << 12, _dma.offset, 0), len);
		}
		_dma.count -= len;
		_dma.offset += len;
	} else {
		if (_silence) {
			memset(stream, 0, _dma.count + 1);
		} else {
			memcpy(stream, &emu_get_memory8(_dma.page << 12, _dma.offset, 0), _dma.count + 1);
		}
		_dma.count = 0xFFFF;
	}

	if (_dma.count == 0xFFFF) {
		_silence = false;
		pic_irq_trigger(7);
	}
}

void emu_io_write_22C(uint8 value)
{
	static uint8 cmd    = 0xFF;
	static bool high    = false;
	static uint16 data  = 0x0000;
	static int old_freq = 0;

	if (!emu_pcm) return;

	switch (cmd) {
		case 0x14:
			data = high ? (data | (value << 8)) : value;
			if (high) {
				SDL_PauseAudio(0);
				cmd = 0xFF;
			}
			high ^= true;
			break;

		case 0x40: {
			SDL_AudioSpec spec;

			spec.freq = 0xF424000 / (0xFFFF - (value << 8));
			spec.format = AUDIO_U8;
			spec.channels = 1;
			spec.samples = 512;
			spec.callback = pcm_sdl_callback;

			if (old_freq != spec.freq) {
				SDL_CloseAudio();
				old_freq = spec.freq;
				SDL_OpenAudio(&spec, &spec);
			}
			cmd = 0xFF;
		} break;

		case 0x80:
			data = high ? (data | (value << 8)) : value;
			if (high) {
				_dma.count = data;
				_silence = true;
				SDL_PauseAudio(0);
				cmd = 0xFF;
			}
			high ^= true;
			break;

		default:
			switch (value) {
				case 0x14:
					cmd = value;
					data = 0;
					high = false;
					break;

				case 0x40:
					cmd = value;
					break;

				case 0x80:
					cmd = value;
					data = 0;
					high = false;
					break;

				case 0xD0:
					SDL_PauseAudio(1);
					break;

				case 0xD3:
					SDL_CloseAudio();
					break;

				default:
					fprintf(stderr, "[PCM] [022C] Not Yet Implemented %02X\n", value);
					break;
			}
	}
}

void emu_io_write_226(uint8 value)
{
	if (value == 0x01) {
		_read_status  = 0x00;
		_read_data    = 0x00;
		_write_status = 0x00;
	} else if (value == 0x00) {
		_read_data    = 0xAA;
		_read_status |= 0x80;
	}
}

void emu_io_write_002(uint8 value)
{
	static bool high = false;

	if (!emu_pcm) return;

	SDL_LockAudio();
	_dma.offset = high ? _dma.offset | (value << 8) : value;
	SDL_UnlockAudio();

	high ^= true;
}

void emu_io_write_003(uint8 value)
{
	static bool high = false;

	if (!emu_pcm) return;

	SDL_LockAudio();
	_dma.count = high ? _dma.count | (value << 8) : value;
	SDL_UnlockAudio();

	high ^= true;
}

uint8 emu_io_read_003()
{
	static bool high = false;
	uint8 value = 0;

	if (!emu_pcm) return value;

	SDL_LockAudio();
	value = high ? (_dma.count >> 8) : (_dma.count & 0xFF);
	SDL_UnlockAudio();

	high ^= true;

	return value;
}

void emu_io_write_083(uint8 value)
{
	if (!emu_pcm) return;

	SDL_LockAudio();
	_dma.page = value;
	SDL_UnlockAudio();
}
