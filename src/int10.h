/* $Id$ */

extern void emu_int10();
extern void emu_int10_gfx(int mode);
extern void emu_int10_update();
extern void emu_int10_uninit(uint8 wait);

extern uint8 emu_io_read_060();
extern uint8 emu_io_read_3C7();
extern uint8 emu_io_read_3C9();
extern uint8 emu_io_read_3D9();
extern uint8 emu_io_read_3DA();

extern void  emu_io_write_3C7(uint8 value);
extern void  emu_io_write_3C8(uint8 value);
extern void  emu_io_write_3C9(uint8 value);
extern void  emu_io_write_3D8(uint8 value);
extern void  emu_io_write_3D9(uint8 value);
