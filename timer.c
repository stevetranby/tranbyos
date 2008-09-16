#include <system.h>

/* This will keep track of how many ticks that the system
*  has been running for */
uint32 	timer_ticks = 0;
uint32	timer_hz = 1000;
uint32 	secondsFromBoot = 0;

uint32 ticks_to_seconds = 1/timer_hz;
//uint32 ticks_to_ms = 1000/timer_hz;
//uint32 ticks_to_us = 1000000/timer_hz;

/* Determine the timer tick rate in Hz. */
void timer_phase(int hz)
{
	timer_hz = hz;
    int divisor = 1193180 / timer_hz; /* Calculate our divisor */
    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. Why 18.222Hz? Some engineer at IBM must've
*  been smoking something funky */
void timer_handler(struct regs *r)
{
    /* Increment our 'tick count' */
    timer_ticks++;    
}

uint32 getTicks() { return timer_ticks; }
uint32 getSeconds() { 
	timer_ticks / ticksToSeconds; 
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{	
    /* Installs 'timer_handler' to IRQ0 */
	irq_install_handler(0, timer_handler);
	timer_phase(100);
}

/* This will continuously loop until the given time has
*  been reached */
void timer_wait(int ticks)
{
	unsigned long eticks;
	
	eticks = timer_ticks + ticks;
	while(timer_ticks < eticks) {}
}

void delay_us(uint32 us) {
	uint32 eticks;	
	eticks = timer_ticks + ticks;
	while(timer_ticks < eticks) {}
}

void delay_ms(uint32 ms) {
	uint32 eticks;	
	eticks = timer_ticks + ticks;
	while(timer_ticks < eticks) {}
}

void delay_s(uint32 s) {
	uint32 eticks;	
	eticks = timer_ticks + ticks;
	while(timer_ticks < eticks) {}
}
