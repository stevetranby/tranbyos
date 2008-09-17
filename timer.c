#include <system.h>

/* This will keep track of how many ticks that the system
*  has been running for */
uint32 _timer_hz = 100;
uint32 _timer_ticks = 0;
uint32 _secondsFromBoot = 0;

#define ticks_from_s(x)  	(x*_timer_hz)
#define ticks_from_ms(x) 	((x*_timer_hz)/1000)
#define ticks_to_s(x)		(x/_timer_hz)
#define ticks_to_ms(x)		((x*1000)/_timer_hz)

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. Why 18.222Hz? Some engineer at IBM must've
*  been smoking something funky */
void timer_handler(struct regs *r)
{
    /* Increment our 'tick count' */
    _timer_ticks++;    
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{	
    /* Installs 'timer_handler' to IRQ0 */
	irq_install_handler(0, timer_handler);
	
	int divisor = 1193180 / _timer_hz; /* Calculate our divisor */
    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

uint32 timer_ticks() { return _timer_ticks; }
uint32 timer_seconds() { return ticks_to_s(_timer_ticks); } 
/* This will continuously loop until the given time has
*  been reached */
void delay_ticks(int ticks)
{
	uint32 eticks = _timer_ticks + ticks;
	while(_timer_ticks < eticks) {}
}

void delay_ms(uint32 ms) {
	uint32 eticks = _timer_ticks + ticks_from_ms(ms);	
	while(_timer_ticks < eticks) {}
}

void delay_s(uint32 s) {
	
	uint32 eticks = _timer_ticks + ticks_from_s(s);
	while(_timer_ticks < eticks) {}
}
