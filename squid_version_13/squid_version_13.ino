#include <EEPROM.h>

/* Done
 TODO: move water sensor to C5
 TODO: move charge sensor to C4
 TODO: move light change output to C3
 TODO: have lure flash its color when removed from charger
 TODO: reset lure when removed from charger?
 TODO: put water sensor in schematic
 
 */

/*
TODO fo realz
 
 
 TODO: put schottkey diode in power path
 put third rail in schematic
 figure out exact values for resistors
 make two versions. One with all LEDs on top. Another with LEDs on bottom too.
 put squid eye on back.
 */


#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

volatile int segment = 0;
volatile int tail = 0;
volatile int tail_delta = 1;
volatile unsigned int counter = 0;

volatile int in_water = 0;

// when this function is called, it is basically a tick. Decide what changes
// are to be executed.
ISR(WDT_vect)
{
  counter++;

  if (!(counter%4))
  {
    //segment++;

    //segment = segment % 5;
    segment = (segment+1) % 5;

  }

  if (!(counter%6))
  {
    tail=tail + tail_delta;

    if (tail == 5) 
      tail_delta = -1; 
    if (tail == 0)
      tail_delta = 1;
  }



}


void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  /* SLEEP_MODE_PWR_SAVE */
  sleep_enable();

  sleep_mode();
  // sleep happens right here
  sleep_disable();

  //power_all_enable();
}




void setup()
{

  power_all_disable();

  //Serial.begin(9600);
  //Serial.println("Initialising...");
  //delay(100); //Allow for serial print to complete.

  DDRB=31; // 0 through 4
  //DDRD=31; // 0 through 4
  DDRD=0xFF; // pins 0 through 5
  DDRC = 1+2+4+8; // tail lights + color selector

  //C5 is water sensor
  //C3 is color setter  

  /*** Setup the WDT ***/
  cli();
  wdt_reset();

  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);

  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  //WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  //WDTCSR = 1<<WDP3; /* 4 seconds */
  //WDTCSR = 1<<WDP2 | 1<< WDP1 | 1<< WDP0; /* 2 seconds */
  //WDTCSR = 1<<WDP2 | 1<<WDP1; /* 1.0 seconds */
  //WDTCSR = 1<<WDP2; //| 1<<WDP0; /* 1/2 seconds */
  //WDTCSR = 1<<WDP2; // 1/4 sec
  //WDTCSR = 1<<WDP0 | 1<<WDP1; // 1/8 second
  //WDTCSR = 1<< WDP1; // 1/16 sec
  WDTCSR = 1<< WDP0; // 1/32 sec

  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);

  sei();

  segment = 0;

  char value = EEPROM.read(0);
  if (value == 'R')
  {
    // turn on RED lights
    EEPROM.write(0, 'G');
    //PINC=1<<3
    PORTC |= (1<<3);
  }
  else //value == 'G'
  {
    //turn on GREEN lights
    EEPROM.write(0, 'R');
    PORTC &= ~(1<<3);
  }

  //toggle color that will turn on 3 times
  for(int i=0; i<3; i++)
  {
    PORTD=1;
    for(int j=0; j<10000; j++)
      __asm__("nop\n\t");
    PORTD=0;
    for(int j=0; j<10000; j++)
      __asm__("nop\n\t");
  }
}

boolean light_on = true;
void loop()
{
  in_water = (PINC>>5) & (0x01);


  if (in_water)
  {
    PORTB = 1<<segment;
    PORTD = 1<<segment;
    PORTC &= ~7; // blank out PORTC which is all the tail lights
    PORTC |= 1<<(tail/2); //why am I doing it this way!!

    int x = tail/2; 
    set x to represent the tail light which should be turned on
      PORTD &= ~(224) // turn off top three bits, which contain yellow lights
      switch(x)
      {
      case 0:
        PORTD |= 1<<5;
        break;
      case 1:
        PORTD |= 1<<6;
        break;
      case 2:
        PORTD |= 1<<7;
        break; 
      }


  }
  else // turn off all lights
  {
    PORTB = PORTD = 0;
    PORTC &=~7; // turn off the first three bits of PORTC
    //WDTCSR = 1<<WDP3; 
  }

  enterSleep(); 
}






