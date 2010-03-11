/* $Id$ */

extern void pic_init();
extern void pic_uninit();
extern void pic_timer_add(void (*callback)(), uint32 usec_delay);
extern void pic_timer_change(void (*callback)(), uint32 usec_delay);
extern void pic_timer_del(void (*callback)());
extern void pic_suspend();
extern void pic_resume();
extern uint32 pic_get_sec();
extern uint32 pic_get_usec();
extern void pic_irq_trigger(uint8 irq);
extern void pic_irq_mask_set(bool pic1, uint8 mask);
extern uint8 pic_irq_mask_get(bool pic1);
extern void pic_irq_command(bool pic1, uint8 command);
