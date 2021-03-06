/* $Id$ */

#include <stdio.h>

/* Windows implementation of the MPU. Uses midiOut functions from the Windows
 *  API, which contain a softsynth and handles all MIDI output for us. */

#if defined(WIN32)

#include <windows.h>
#include "types.h"
#include "libemu.h"
#include "mpu.h"

static HMIDIOUT _midi = NULL;

void mpu_init()
{
	if (midiOutOpen(&_midi, 0, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
		fprintf(stderr, "[EMU] Failed to initialize MPU\n");
		_midi = NULL;
		return;
	}
}

void mpu_uninit()
{
	if (_midi == NULL) return;

	midiOutReset(_midi);
	midiOutClose(_midi);

	_midi = NULL;
}

void mpu_send(uint32 data)
{
	if (_midi == NULL) return;

	midiOutShortMsg(_midi, data);
}

void mpu_reset()
{
	if (_midi == NULL) return;

	midiOutReset(_midi);
}

#endif /* WIN32 */
