/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
#include <stdio.h>



#define MAX_PWM_VAL 1024


#define millis()  (SysTick->CNT / DELAY_MS_TIME)

const uint16_t CIE[256] = {
    0,    0,    1,    1,    2,    2,    3,    3,    4,    4,    4,    5,    5,    6,    6,    7,
    7,    8,    8,    8,    9,    9,   10,   10,   11,   11,   12,   12,   13,   13,   14,   15,
   15,   16,   17,   17,   18,   19,   19,   20,   21,   22,   22,   23,   24,   25,   26,   27,
   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,   42,   43,   44,
   45,   47,   48,   50,   51,   52,   54,   55,   57,   58,   60,   61,   63,   65,   66,   68,
   70,   71,   73,   75,   77,   79,   81,   83,   84,   86,   88,   90,   93,   95,   97,   99,
  101,  103,  106,  108,  110,  113,  115,  118,  120,  123,  125,  128,  130,  133,  136,  138,
  141,  144,  147,  149,  152,  155,  158,  161,  164,  167,  171,  174,  177,  180,  183,  187,
  190,  194,  197,  200,  204,  208,  211,  215,  218,  222,  226,  230,  234,  237,  241,  245,
  249,  254,  258,  262,  266,  270,  275,  279,  283,  288,  292,  297,  301,  306,  311,  315,
  320,  325,  330,  335,  340,  345,  350,  355,  360,  365,  370,  376,  381,  386,  392,  397,
  403,  408,  414,  420,  425,  431,  437,  443,  449,  455,  461,  467,  473,  480,  486,  492,
  499,  505,  512,  518,  525,  532,  538,  545,  552,  559,  566,  573,  580,  587,  594,  601,
  609,  616,  624,  631,  639,  646,  654,  662,  669,  677,  685,  693,  701,  709,  717,  726,
  734,  742,  751,  759,  768,  776,  785,  794,  802,  811,  820,  829,  838,  847,  857,  866,
  875,  885,  894,  903,  913,  923,  932,  942,  952,  962,  972,  982,  992, 1002, 1013, 1023,
};

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
 * set timer channel PW
 */
void t1pwm_setpw(uint8_t chl, uint16_t width)
{
	// The pulse apparently never ends if it is maximum length!
	if (width == MAX_PWM_VAL) {
		width = MAX_PWM_VAL-1;
	}
	switch(chl&3)
	{
		case 0: TIM1->CH1CVR = width; break;
		case 1: TIM1->CH2CVR = width; break;
		case 2: TIM1->CH3CVR = width; break;
		case 3: TIM1->CH4CVR = width; break;
	}
}


#define NUM_LEDS 3
const int led_pins[NUM_LEDS] = {9, 5, 6};
const int led_periods_ms[NUM_LEDS] = {500, 505, 510};
uint8_t i = 0;
const int dead_time_half = 500 / 8 / 2;

void LEDBeats() {

  for (i = 0; i < NUM_LEDS; i++) {
		// Start them flashing at the same time
    int timestamp = (millis() + 50) % led_periods_ms[i];
    
    int pwm_value = 0;
    if (timestamp > led_periods_ms[i] / 2) {
      // Make pattern symmetric at half of the period
      timestamp = led_periods_ms[i] - timestamp;
    }

		if (timestamp < dead_time_half) {
			pwm_value = 0;
		} else {

			pwm_value = map(timestamp, dead_time_half, led_periods_ms[i] / 2, 0, 255);
			pwm_value = CIE[pwm_value];
		}
        
    t1pwm_setpw(i, MAX_PWM_VAL-pwm_value);
  }
  
}

/*
 * initialize TIM1 for PWM
 */
void t1pwm_init( void )
{
	// Enable GPIOC, GPIOD, GPIOA and TIM1
	RCC->APB2PCENR |= 	RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA |
						RCC_APB2Periph_TIM1;
	
	// PD2 is T1CH1, 10MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf<<(4*2));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*2);
	
	// PA1 is T1CH2, 10MHz Output alt func, push-pull
	GPIOA->CFGLR &= ~(0xf<<(4*1));
	GPIOA->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*1);

	// PC3 is T1CH3, 10MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*3));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*3);

	// PC4 is T1CH4, 10MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);


/*
	// PC0 is T1CH3 alternate mapping, 10MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*0);
	AFIO->PCFR1 |= GPIO_PartialRemap1_TIM1;
*/

		
	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
	
	// CTLR1: default is up, events generated, edge align
	// SMCFGR: default clk input is CK_INT
	
	// Prescaler 
	TIM1->PSC = 0x6;
	
	// Auto Reload - sets period
	TIM1->ATRLR = MAX_PWM_VAL;
	
	// Reload immediately
	//TIM1->SWEVGR |= TIM_UG;
	// One pulse mode, fire an interrupt when pulse finishes
	TIM1->CTLR1 |= TIM_OPM | TIM_URS;
	
	// Enable CH1 output, positive pol
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;

	// Enable CH2 output, positive pol
	TIM1->CCER |= TIM_CC2E | TIM_CC2P;

	// Enable CH3 output, positive pol
	TIM1->CCER |= TIM_CC3E | TIM_CC3P;
	
	// Enable CH4 output, positive pol
	TIM1->CCER |= TIM_CC4E | TIM_CC4P;
	
	// CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;
	TIM1->CHCTLR1 |= TIM_OC2M_2 | TIM_OC2M_1;
	
	// CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1;
	TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1;
	
	// Set the Capture Compare Register value to 0% initially
	TIM1->CH1CVR = MAX_PWM_VAL;
	TIM1->CH2CVR = MAX_PWM_VAL;
	TIM1->CH3CVR = MAX_PWM_VAL;
	TIM1->CH4CVR = MAX_PWM_VAL;
	
	// Enable TIM1 outputs
	TIM1->BDTR |= TIM_MOE;
	
	// Enable TIM1
	TIM1->CTLR1 |= TIM_CEN;
}


/*
// Set up an interrupt handler for the update event (counter flips to 0)

void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void TIM1_UP_IRQHandler(void)
{
	// move the compare further ahead in time.
	// as a warning, if more than this length of time
	// passes before triggering, you may miss your
	// interrupt.
	SysTick->CMP += (FUNCONF_SYSTEM_CORE_CLOCK/1000);

	// clear IRQ
	SysTick->SR = 0;

	// update counter
	systick_cnt++;
}

*/




int main()
{
	
	SystemInit();
	Delay_Ms( 1000 );

	//RCC->CFGR0 &= RCC_HPRE;
	// RCC->CFGR0 |= RCC_HPRE_DIV32;


	printf("\r\r\n\ntim1_pwm example\n\r");

	// init TIM1 for PWM
	printf("initializing tim1...");
	t1pwm_init();
	printf("done.\n\r");
		

	printf("looping...\n\r");
	//AFIO->PCFR1 |= GPIO_PartialRemap1_TIM1;
	while(1) {
		
			

		// Determine next values of LEDs. 
		// Theoretically only MAX_PWM_VAL ticks of CPU available, but can 
		// increase cpu freq if needed
		LEDBeats();

		TIM1->CTLR1 |= TIM_CEN;
		// Wait until TIM1 is done with pulse
		// Optionally sleep CPU and have end of pulse automatically go to deep sleep?
		while (TIM1->CTLR1 & TIM_CEN);
		//Delay_Us( delay );

		// TODO: Sleep!
		// For now just do delay
		t1pwm_setpw(1, MAX_PWM_VAL); // Chl 1
		Delay_Us( 13333 );
	}	
}

// TODO before ship:
//  * Set prescalar for TIM1 to 0 and adjust system clock. Need to make sure millis() still works. Propose millis()?
//  * Clean up unused code for clarity
//  * Determine if more CPU time is needed and increase CPU freq and MAX_PWM_VAL to compensate. Still want ~.4 seconds of light in the end though.