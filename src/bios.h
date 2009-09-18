
enum {
	TIMER_MEMORY_PAGE  = 0xE000,   //  7 * 16 bytes
	MOUSE_MEMORY_PAGE  = 0xE100,   //  1 * 16 bytes
	VGADAC_MEMORY_PAGE = 0xE200,   // 65 * 16 bytes

	BIOS_MEMORY_PAGE = 0x40,

	BIOS_EQUIPEMENT = 0x10,        // 2 bytes

	BIOS_KEYBOARD_SHIFT = 0x17,    // 1 byte
	BIOS_KEYBOARD_TOGGLE = 0x18,   // 1 byte

	BIOS_VIDEO_MODE = 0x49,        // 1 byte

	BIOS_COUNTER = 0x6C,           // 4 bytes
	BIOS_COUNTER_OVERFLOW = 0x70,  // 1 byte

	BIOS_VIDEO_EGA_INFO = 0x87,    // 1 byte
	BIOS_VIDEO_EGA_SWITCHES = 0x88,// 1 byte
};

extern void bios_init();
extern void bios_uninit(int exitCode);
