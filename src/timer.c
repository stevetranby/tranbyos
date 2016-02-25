#include "include/system.h"

/* This will keep track of how many ticks that the system
*  has been running for */
u32 _timer_hz = 100;
u32 _timer_ticks = 0;
u32 _secondsFromBoot = 0;

#define ticks_from_s(x)  	(x*_timer_hz)
#define ticks_from_ms(x) 	((x*_timer_hz)/1000)
#define ticks_to_s(x)		(x/_timer_hz)
#define ticks_to_ms(x)		((x*1000)/_timer_hz)

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. Why 18.222Hz? Some engineer at IBM must've
*  been smoking something funky */
void timer_handler(isr_stack_state *r)
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

u32 timer_ticks() { return _timer_ticks; }
u32 timer_seconds() { return ticks_to_s(_timer_ticks); } 
/* This will continuously loop until the given time has
*  been reached */
void delay_ticks(int ticks)
{
	u32 eticks = _timer_ticks + ticks;
	while(_timer_ticks < eticks) {}
}

void delay_ms(u32 ms) {
	u32 eticks = _timer_ticks + ticks_from_ms(ms);	
	while(_timer_ticks < eticks) {}
}

void delay_s(u32 s) {
	
	u32 eticks = _timer_ticks + ticks_from_s(s);
	while(_timer_ticks < eticks) {}
}


////////////////////////////////////////////////////

// http://wiki.osdev.org/CMOS#Getting_Current_Date_and_Time_from_RTC
// http://www.bioscentral.com/misc/cmosmap.htm
// Reading date from CMOS


//Reading All RTC Time and Date Registers
const u16 kCurrentYear = 2014;

// TODO: Change this each year!
  
// Set by ACPI table parsing code if possible
int century_register = 0x00;                                
 
enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};


int get_update_in_progress_flag() {
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
      u8 year;
      u8 century;
      
      u8 last_second;
      u8 last_minute;
      u8 last_hour;
      u8 last_day;
      u8 last_month;
      u8 last_year;
      u8 last_century;
      
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
 
      if (!(registerB & 0x04)) {
            second = (second & 0x0F) + ((second / 16) * 10);
            minute = (minute & 0x0F) + ((minute / 16) * 10);
            hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
            day = (day & 0x0F) + ((day / 16) * 10);
            month = (month & 0x0F) + ((month / 16) * 10);
            year = (year & 0x0F) + ((year / 16) * 10);
            if(century_register != 0) {
                  century = (century & 0x0F) + ((century / 16) * 10);
            }
      }
 
      // Convert 12 hour clock to 24 hour clock if necessary
 
      if (!(registerB & 0x02) && (hour & 0x80)) {
            hour = ((hour & 0x7F) + 12) % 24;
      }
 
      // Calculate the full (4-digit) year
 
      if(century_register != 0) {
            year += century * 100;
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