#include <system.h>

/* This will keep track of how many ticks that the system
*  has been running for */
int 	timer_ticks = 0;
int		timer_hz = 18;
uint32 	secondsFromBoot = 0;

/* Determine the timer tick rate in Hz. */
void timer_phase(int hz)
{
	timer_hz = hz;
    int divisor = 1193180 / timer_hz; /* Calculate our divisor */
    outportb(0x43, 0x36);             /* Set our command byte 0x36 */
    outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
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

    if (timer_ticks % timer_hz == 0)
    {
		secondsFromBoot++;
		#ifdef DEBUG
			putch('.');
		#endif
    }
}

uint32 time_ticks() { return timer_ticks; }
uint32 time_s() { return secondsFromBoot; }

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
	while(timer_ticks < eticks);
}
