#include <system.h>

///////////////////////////////////////////////////////////////////

#define ticks_from_s(x)  	(x * _timer_hz)
#define ticks_from_ms(x) 	((x * _timer_hz) / 1000)
#define ticks_to_s(x)		(x / _timer_hz)
#define ticks_to_ms(x)		((x * 1000) / _timer_hz)

#define PIT_PORT    0x40
#define PIT_CTRL    0x43
#define PIT_CMD_SET 0x36
#define PIT_MASK    0xFF

// Our desired timer properties
// Default interrupt freq 18.222Hz
static u32 _timer_hz = 100;
static u32 _timer_ticks = 0;
// static u32 _secondsFromBoot = 0;
// static u32 _timer_subticks = 0;
// static i32 _timer_drift = 0;

// Scheduling Tasks
static u32 _next_preempt = 0;
static u32 _ticks_per_schedule = 50;

u32 timer_ticks() {
    return _timer_ticks;
}

u32 timer_seconds() {
    return ticks_to_s(_timer_ticks);
}

// IRQ handler
void timer_handler(isr_stack_state *r)
{
    UNUSED_PARAM(r);
    ++_timer_ticks;
    //trace("timer ticks: %d\n", _timer_ticks);

   // schedule tasks
   if(_next_preempt < _timer_ticks) {
       _next_preempt = _timer_ticks + _ticks_per_schedule;
       trace("preempting to next task\n");
       k_preempt_kernel();
   }
}

// Install IRQ Handler
void timer_install()
{
    irq_install_handler(0, timer_handler, "timer");

    int divisor = 1193180 / _timer_hz; /* Calculate our divisor */
    outb(PIT_CTRL, PIT_CMD_SET);             /* Set our command byte 0x36 */
    outb(PIT_PORT, divisor & PIT_MASK);   /* Set low byte of divisor */
    outb(PIT_PORT, (divisor >> 8) & PIT_MASK);     /* Set high byte of divisor */
}

/* This will continuously loop until the given time has
 *  been reached */
void delay_ticks(i32 ticks)
{
    u32 eticks = _timer_ticks + ticks;
    while(_timer_ticks < eticks) {}
}

// TODO: figure out why this fails on gcc optimization flag -O2
void delay_ms(u32 ms) {
    trace_info("[enter] delay_ms\n");
    u32 eticks = _timer_ticks + ticks_from_ms(ms);
    trace_info("test: %d < %d\n", _timer_ticks, eticks);
    while(_timer_ticks < eticks) {
        //trace("test: %d < %d\n", _timer_ticks, eticks);

        // TODO: Try prevent O2 optimize away
        while(0) { asm(""); }
    }
    trace_info("[exit] delay_ms, %d < %d\n", _timer_ticks, eticks);
}

void delay_s(u32 s) {

    u32 eticks = _timer_ticks + ticks_from_s(s);
    while(_timer_ticks < eticks) {}
}


//////////////////////////////////////////////////////////////////

// http://wiki.osdev.org/CMOS#Getting_Current_Date_and_Time_from_RTC
// http://www.bioscentral.com/misc/cmosmap.htm
// Reading date from CMOS


//Reading All RTC Time and Date Registers
const u32 kCurrentYear = 2016;

// TODO: Change this each year!

// Set by ACPI table parsing code if possible
int century_register = 0x00;

enum {
    cmos_address = 0x70,
    cmos_data    = 0x71
};

void write_to_CMOS(u8 data[])
{
    for(u8 index = 0; index < 128; index++)
    {
        // when dealing with CMOS we want to make sure nothing interrupts the write process
        cli();
        outb(cmos_address, index);
        outb(cmos_data, data[index]);
        sti();
    }
}

u8 get_update_in_progress_flag() {
    outb(cmos_address, 0x0A);
    return (inb(cmos_data) & 0x80);
}

u8 get_RTC_register(int reg) {
    outb(cmos_address, reg);
    return inb(cmos_data);
}

rtc_time read_rtc() {

    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u32 year;
    u32 century = 0;

    u8 last_second;
    u8 last_minute;
    u8 last_hour;
    u8 last_day;
    u8 last_month;
    u32 last_year;
    u32 last_century = 0;

    u8 registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates

    while (get_update_in_progress_flag());                // Make sure an update isn't in progress

    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);

    if(century_register != 0) {
        century = get_RTC_register(century_register);
    }

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;

        while (get_update_in_progress_flag());           // Make sure an update isn't in progress

        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);

        if(century_register != 0) {
            century = get_RTC_register(century_register);
        }
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
            (last_day != day) || (last_month != month) || (last_year != year) ||
            (last_century != century) );

    registerB = get_RTC_register(0x0B);

    // Convert BCD to binary values if necessary
    kputs("year = ");
    printHex(year);
    kputs("\n");
    kputs("century = ");
    printHex(century);
    kputs("\n");
    kputs("century_register = ");
    printHex(century_register);
    kputs("\n");
    kputs("registerB = ");
    printHex(registerB);
    kputs("\n");

    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);

        // 0x16 -> 0x06 + 10
        year = (year & 0x0F) + ((year / 16) * 10);

        if(century_register != 0) {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    // Convert 12 hour clock to 24 hour clock if necessary

    kputs("hour(before 24hr check) = ");
    printInt(hour);
    kputs("\n");
    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year
    if(century_register != 0) {
        year += century * 100;
        kputs("century_register != ZERO!");
    } else {
        year += (kCurrentYear / 100) * 100;
        if(year < kCurrentYear) year += 100;
    }

    rtc_time ret;
    ret.year = year;
    ret.month = month;
    ret.day = day;
    ret.hour = hour;
    ret.minute = minute;
    ret.second = second;
    return ret;
}