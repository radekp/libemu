#include <stdio.h>
#include "types.h"
#include "libemu.h"
#include "bios.h"
#include "pic.h"
#include "timer.h"
#include "libemu.h"

#define COUNTER_TO_USEC 1000 / 1193
#define USEC_TO_COUNTER 1193 / 1000

MSVC_PACKED_BEGIN
typedef struct Timer {
	uint8 bcd;      // 0 = 16bit, 1 = BCD
	uint8 access;   // 1 = MSB only, 2 = LSB only, 3 = LSB first, then MSB, 4 = MSB (after LSB)
	uint8 mode;
	uint8 start;
	uint32 read;
	uint32 write;
	uint32 delay;
	uint32 last_sec;
	uint32 last_usec;
	uint8 unused[8];
} GCC_PACKED Timer;
MSVC_PACKED_END

static Timer *_timer;

void _timer_run()
{
	if (emu_flags.inf) emu_hard_int(0x08);
}

void _timer_pic_prepare_read(uint8 counter)
{
	uint32 new_sec            = pic_get_sec();
	uint32 new_usec           = pic_get_usec();
	uint32 usec_delta         = (new_sec - _timer[counter].last_sec) * 1000000 + (new_usec - _timer[counter].last_usec);
	_timer[counter].last_sec  = new_sec;
	_timer[counter].last_usec = new_usec;

	if (!_timer[counter].start) return;

	switch (_timer[counter].mode) {
		case 0: // Countdown, Interrupt, Stop
		case 4: // Software Triggered Strobe
			_timer[counter].read = (_timer[counter].delay + _timer[counter].read - usec_delta) % _timer[counter].delay;
			if (usec_delta > _timer[counter].read) {
				/* We passed the timer, so we need to stop this counter */
				_timer[counter].start = 0;
				_timer[counter].read  = 0;
			}
			break;

		case 2: // Rate Generate
			_timer[counter].read = (_timer[counter].delay + _timer[counter].read - usec_delta) % _timer[counter].delay;
			break;

		case 3: // Square Wave Rate Generate, runs twice as fast
			_timer[counter].read = (_timer[counter].delay + _timer[counter].read - usec_delta * 2) % _timer[counter].delay;
			if (_timer[counter].read & 0x1) _timer[counter].read -= 1;
			break;

		case 1:
		case 5:
		default:
			_timer[counter].read = 0xFFFF;
			return; // Return, as we don't need to convert to a counter
	}

	_timer[counter].read = _timer[counter].read * USEC_TO_COUNTER;
}

uint8 _timer_pic_read(uint8 counter)
{
	switch (_timer[counter].access) {
		case 1: return (_timer[counter].read >> 0) & 0xFF;
		case 2: return (_timer[counter].read >> 8) & 0xFF;
		case 3: _timer[counter].access = 4; return (_timer[counter].read >> 0) & 0xFF;
		case 4: _timer[counter].access = 3; return (_timer[counter].read >> 8) & 0xFF;
		default: return 0xFF;
	}
}

void _timer_pic_write(uint8 counter, uint8 value)
{
	switch (_timer[counter].access) {
		case 1: _timer[counter].write = (value << 0); break;
		case 2: _timer[counter].write = (value << 8); break;
		case 3: _timer[counter].access = 4; _timer[counter].write  = (value << 0); return; // First a MSB before we change anything
		case 4: _timer[counter].access = 3; _timer[counter].write |= (value << 8); break;
	}
	if (_timer[counter].write == 0x0) _timer[counter].write = (_timer[counter].bcd) ? 9999 : 0x10000;
	_timer[counter].delay = _timer[counter].write * COUNTER_TO_USEC;

	if (counter == 0) pic_timer_change(_timer_run, _timer[0].delay);
}

void _timer_pic_setting(uint8 setting)
{
	uint8 counter = (setting >> 6) & 0x03;
	if (counter == 0x03) return; // We only implement the 8253, not 8254

	_timer[counter].bcd = ((setting & 0x01) != 0) ? 1 : 0;

	uint8 access = (setting >> 4) & 0x03;
	if (access == 0) {
		_timer_pic_prepare_read(counter);
		return;
	}

	/* Timer is reprogrammed */
	_timer[counter].access = access;
	_timer[counter].mode   = (setting >> 1) & 0x07;
	_timer[counter].start  = 1;
	if (_timer[counter].mode > 0x05) _timer[counter].mode -= 0x04; // Mode 6 and 7 don't exist, convert to 2 and 3
}

void timer_init()
{
	/* Link the timers to the memory */
	_timer = (Timer *)&emu_get_memory8(TIMER_MEMORY_PAGE, 0, 0x10);

	if (emu_get_memory8(TIMER_MEMORY_PAGE, 0, 0) == 0x00) {
		emu_get_memory8(TIMER_MEMORY_PAGE, 0, 0) = 0xFF;

		/* Initialize all timers */
		_timer[0].bcd     = 0;
		_timer[0].delay   = 0x10000 * COUNTER_TO_USEC;
		_timer[0].access  = 3;
		_timer[0].mode    = 3;

		_timer[1].bcd     = 0;
		_timer[1].delay   = 0x12 * COUNTER_TO_USEC;
		_timer[1].access  = 1;
		_timer[1].mode    = 2;

		_timer[2].bcd     = 0;
		_timer[2].delay   = 0x528 * COUNTER_TO_USEC;
		_timer[2].access  = 3;
		_timer[2].mode    = 3;

		/* Set the starting time of all timer */
		int i;
		uint32 start_sec  = pic_get_sec();
		uint32 start_usec = pic_get_usec();
		for (i = 0; i < 3; i++) {
			_timer[i].start     = 1;
			_timer[i].read      = _timer[i].delay;
			_timer[i].last_sec  = start_sec;
			_timer[i].last_usec = start_usec;
		}
	} else {
		/* Make sure the timers don't detect a big jump forward in timing */
		int i;
		uint32 start_sec  = pic_get_sec();
		uint32 start_usec = pic_get_usec();
		for (i = 0; i < 3; i++) {
			_timer[i].last_sec  = start_sec;
			_timer[i].last_usec = start_usec;
		}
	}

	pic_timer_add(_timer_run, _timer[0].delay);
}

void timer_uninit()
{
	pic_timer_del(_timer_run);
}

uint8 emu_io_read_040()
{
	return _timer_pic_read(0);
}

uint8 emu_io_read_042()
{
	return _timer_pic_read(2);
}

void emu_io_write_040(uint8 value)
{
	_timer_pic_write(0, value);
}

void emu_io_write_042(uint8 value)
{
	_timer_pic_write(2, value);
}

void emu_io_write_043(uint8 value)
{
	_timer_pic_setting(value);
}
