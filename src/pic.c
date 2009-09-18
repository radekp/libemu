#include <stdio.h>
#include <stdlib.h>
#if !defined(_MSC_VER)
#	include <sys/time.h>
#endif /* _MSC_VER */
#if defined(WIN32)
#	include <windows.h>
#else
#	include <signal.h>
#endif /* WIN32 */
#include "types.h"
#include "pic.h"

typedef struct PICNode {
	uint32 usec_left;
	uint32 usec_delay;
	void (*callback)();
} PICNode;

#if defined(WIN32)
static HANDLE _timer_thread = NULL;
static HANDLE _timer_event  = NULL;
static HANDLE _timer_main   = NULL;
static int _timer_in_thread = 0;
static int _timer;
#else
static struct itimerval _timer;
#endif /* WIN32 */
static PICNode *_pic_nodes = NULL;
static int _pic_node_count = 0;
static int _pic_node_size  = 0;
static uint32 _pic_last_sec;
static uint32 _pic_last_usec;
const uint32 _pic_speed    = 20000; // Our PIC runs at 50Hz

void _pic_run()
{
	/* Lock the PIC, to avoid double-calls */
	static uint8 pic_running = 0;
	if (pic_running == 1) return;
	pic_running = 1;

	/* Calculate the time between calls */
	uint32 new_sec    = pic_get_sec();
	uint32 new_usec   = pic_get_usec();
	uint32 usec_delta = (new_sec - _pic_last_sec) * 1000000 + (new_usec - _pic_last_usec);
	_pic_last_sec     = new_sec;
	_pic_last_usec    = new_usec;

	/* Walk all our timers, see which (and how often) it should be triggered */
	uint32 delta;
	int i;
	PICNode *node = _pic_nodes;
	for (i = 0; i < _pic_node_count; i++, node++) {
		delta = usec_delta;

		/* No delay means: as often as possible, but don't worry about it */
		if (node->usec_delay == 0) {
			node->callback();
			continue;
		}

		while (node->usec_left < delta) {
			delta -= node->usec_left;
			node->usec_left = node->usec_delay;
			node->callback();
		}
		node->usec_left -= delta;
	}

	pic_running = 0;
}

#if defined(WIN32)
void pic_windows_thread(void *arg) {
	while (WaitForSingleObject(_timer_event, _timer) == WAIT_TIMEOUT) {
		_timer_in_thread = 1;
		SuspendThread(_timer_main);
		_pic_run();
		ResumeThread(_timer_main);
		_timer_in_thread = 0;
	}
}
#endif /* WIN32 */

void pic_init()
{
	_pic_last_sec  = pic_get_sec();
	_pic_last_usec = pic_get_usec();

#if defined(WIN32)
	_timer = _pic_speed / 1000;
#else
	_timer.it_value.tv_sec = 0;
	_timer.it_value.tv_usec = _pic_speed;
	_timer.it_interval.tv_sec = 0;
	_timer.it_interval.tv_usec = _pic_speed;

	signal(SIGALRM, _pic_run);
#endif /* WIN32 */
	pic_resume();
}

void pic_uninit()
{
	pic_suspend();
}

void pic_timer_add(void (*callback)(), uint32 usec_delay)
{
	if (_pic_node_count == _pic_node_size) {
		_pic_node_size += 2;
		_pic_nodes = (PICNode *)realloc(_pic_nodes, _pic_node_size * sizeof(PICNode));
	}
	PICNode *node = &_pic_nodes[_pic_node_count++];

	node->usec_left  = usec_delay;
	node->usec_delay = usec_delay;
	node->callback   = callback;
}

void pic_timer_change(void (*callback)(), uint32 usec_delay)
{
	int i;
	PICNode *node = _pic_nodes;
	for (i = 0; i < _pic_node_count; i++, node++) {
		if (node->callback == callback) {
			node->usec_delay = usec_delay;
			return;
		}
	}
}

void pic_timer_del(void (*callback)())
{
	int i;
	PICNode *node = _pic_nodes;
	for (i = 0; i < _pic_node_count; i++, node++) {
		if (node->callback == callback) {
			*node = _pic_nodes[--_pic_node_count];
			return;
		}
	}
}

void pic_suspend()
{
#if defined(WIN32)
	if (_timer_thread != NULL) {
		if (_timer_in_thread == 0) SuspendThread(_timer_thread);
	}
#else
	setitimer(ITIMER_REAL, NULL, NULL);
#endif /* WIN32 */
}

void pic_resume()
{
#if defined(WIN32)
	if (_timer_thread == NULL) {
		DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &_timer_main, 0, FALSE, DUPLICATE_SAME_ACCESS);
		_timer_event  = CreateEvent(NULL, FALSE, FALSE, NULL);
		_timer_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pic_windows_thread, NULL, 0, NULL);
	} else {
		if (_timer_in_thread == 0) ResumeThread(_timer_thread);
	}
#else
	setitimer(ITIMER_REAL, &_timer, NULL);
#endif /* WIN32 */
}

uint32 pic_get_sec()
{
#if defined(_MSC_VER)
	DWORD t;
	t = timeGetTime();
	return t / 1000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec;
#endif /* _MSC_VER */
}

uint32 pic_get_usec()
{
#if defined(_MSC_VER)
	DWORD t;
	t = timeGetTime();
	return (t % 1000) * 1000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec;
#endif /* _MSC_VER */
}
