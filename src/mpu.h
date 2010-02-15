/* $Id$ */

extern void mpu_init();
extern void mpu_uninit();
extern void mpu_send(uint32 data);
extern void mpu_reset();

extern uint8 emu_io_read_330();
extern void emu_io_write_330(uint8 value);
extern uint8 emu_io_read_331();
extern void emu_io_write_331(uint8 value);
