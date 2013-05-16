// Evan Sapienza
// 2-3-13
// Reaction Timer
// SMCLK drives WDT which divides down by 8192
// WDT handler toggles output pin P1.1 which can drive a scope or audio
//******************************************************************************

// the following line loads definitions for the
#include  <msp430g2553.h>

// mask to select P1.1 as the output pin
#define RED BIT0
#define BUTTON BIT3
#define GREEN BIT6


unsigned int blink_interval;  // number of WDT interrupts per blink of LED
volatile unsigned int blink_counter;   // down counter for interrupt handler
unsigned char state;    // current state of the application
unsigned char last_button_state;   // state of the button the last time it was read
double timer;
int green, red, counter, start, timer_state;


// ===== Main Program (System Reset Handler) =====
void main(void)
{
  BCSCTL1 = CALBC1_1MHZ;	// 1 Mhz calibration for clock initially
  DCOCTL  = CALDCO_1MHZ;

  // setup the watchdog timer as an interval timer
  WDTCTL =(WDTPW +	// (bits 15-8) password
                        // bit 7=0 => watchdog timer on
                        // bit 6=0 => NMI on rising edge (not used here)
                        // bit 5=0 => RST/NMI pin does a reset (not used here)
           WDTTMSEL +   // (bit 4) select interval timer mode
           WDTCNTCL + 	// (bit 3) clear watchdog timer counter
  		                // bit 2=0 => SMCLK is the source
  		   1            // bits 1-0 = 01 => source/8K .
  		   );
  IE1 |= WDTIE;		// enable the WDT interrupt (in the system interrupt register IE1)

  // initialize the I/O port.  Pins 0-1-2 are used
    P1DIR |= (RED+GREEN);     // Set P1.0, P1.3 is input (the default)
    P1REN = BUTTON;  // enable internal 'PULL' resistor for the button
    P1OUT = BUTTON;  // pull up

    // initialize the state variables
    blink_interval=100; // corresponds to about 1/2 sec
    blink_counter=blink_interval;
    timer=0;
    green = 0;
    red = 0;
    counter = 0;
    start = 1;
    timer_state = 0;
    last_button_state=(P1IN&BUTTON);
 // in the CPU status register, set bits:
 //    GIE == CPU general interrupt enable
 //    LPM0_bits == bit to set for CPUOFF (which is low power mode 0)

 _bis_SR_register(GIE+LPM0_bits);  // after execution of this instruction, the CPU is off!
}

// ===== Watchdog Timer Interrupt Handler =====
// This event handler is called to handle the watchdog timer interrupt,
//    which is occurring regularly at intervals of 32K/8MHz ~= 4ms.


interrupt void WDT_interval_handler(){
	 char current_button;
	  // poll the button to see if we need to change state
	  current_button=(P1IN & BUTTON); // read button bit

	  if (start==1 && (current_button==0) && last_button_state){ // did the button go down?
		  green = 1;
		  start = 0;
	  }

	  if ((green == 1) && (counter < 4) && (--blink_counter==0)){
		  	  blink_counter=blink_interval; // reset the down counter
			  P1OUT ^= GREEN;// toggle LED
			  counter++;
			  if (counter >= 4){
				  green = 0;
				  red = 1;
				  counter = 0;
			  }
		  }

	  if ((red == 1) && (counter < 2) && (--blink_counter==0)){
		  	  P1OUT &= ~GREEN;
			  P1OUT ^= RED;// toggle LED
			  blink_counter=blink_interval; // reset the down counter
			  counter++;
			  if (counter >= 2){
				  red = 0;
				  timer = 0;
				  timer_state = 1;
			  }
		  }

	  if (timer_state==1){
		  timer++;
		  if ((current_button==0) && last_button_state){
			  start = 1;
			  timer = timer * 7.4;
			  timer_state = 0;
			  counter = 0;
		  }
	  }

	  last_button_state=current_button; // remember the new button state
}

// DECLARE WDT_interval_handler as handler for interrupt 10
ISR_VECTOR(WDT_interval_handler, ".int10")




