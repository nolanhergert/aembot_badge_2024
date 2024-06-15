/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
// Override default values for clock division
	#define BASE_CFGR0_ORIG RCC_HPRE_DIV1 | RCC_PLLSRC_HSI_Mul2
	#define BASE_CFGR0_NEW RCC_HPRE_DIV16 | RCC_PLLSRC_HSI_Mul2
	//#define HSI_VALUE          (1500000) //
#include <stdio.h>
#include "ch32v003_GPIO_branchless.h"

// Are we going to use deep sleep? If yes, leave uncommented
#define DEEP_SLEEP

#define DEEP_SLEEP_TIME_MS 17 // (really 16.8)

#define MAX_PWM_VAL 1024

#define NUM_LEDS 3
uint8_t i = 0;

uint8_t button_is_pressed = 0;
long button_started_press_ms = 0;
long button_released_press_ms = 0;
#define BUTTON_DEBOUNCE_DELAY_MS 20

long millis_start = 0;
long deep_sleep_time_ms = 0;
// Time in milliseconds since the board was turned on
// Compensates for startup delay and deep sleep section, where the main HSI clock
// is not ticking!
#define millis()  (SysTick->CNT / DELAY_MS_TIME - millis_start + deep_sleep_time_ms)

// From https://gist.github.com/mathiasvr/19ce1d7b6caeab230934080ae1f1380e
#define MAX_CIE_INDEX (256-1)
const uint16_t CIE[MAX_CIE_INDEX+1] = {
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
	// Make values make sense. 0 = dark, MAX_PWM_VAL = bright
	width = MAX_PWM_VAL - width;
	switch(chl&3)
	{
		case 0: TIM1->CH1CVR = width; break;
		case 1: TIM1->CH3CVR = width; break;
		case 2: TIM1->CH4CVR = width; break;
	}
}

uint16_t next_pwm_vals[NUM_LEDS] = {0};

const long ms_in_minute = 60000;
const long led_pulse_periods_ms[NUM_LEDS] = {ms_in_minute/50.4, ms_in_minute/50.2, ms_in_minute/50};

void LEDBeats() {

  for (i = 0; i < NUM_LEDS; i++) {
		// Start them flashing at the same time
    int timestamp = (millis()) % led_pulse_periods_ms[i];

    int pwm_value = 0;

		if (timestamp < led_pulse_periods_ms[i]/4) {
			pwm_value = map(timestamp, 0, led_pulse_periods_ms[i]/4, 255, 0);
		} else {
			pwm_value = 0;
		}

		// Map "linear" to logarithmic value to match human vision
    pwm_value = CIE[pwm_value];
		next_pwm_vals[i] = pwm_value;
  }
}

const long breath_period_ms = 10*1000;
void Breathe() {
	long timestamp = (millis()) % breath_period_ms;

	int pwm_value = 0;

	if (timestamp < breath_period_ms/3) {
		// Breathe in
		pwm_value = map(timestamp, 0, breath_period_ms/3, 0, 255);
	} else if (timestamp < (2*breath_period_ms)/3) {
		// Breathe out
		pwm_value = map(timestamp, breath_period_ms/3, (2*breath_period_ms)/3, 255, 0);
	} else {
		pwm_value = 0;
	}

	// Map "linear" to logarithmic value to match human vision
	pwm_value = CIE[pwm_value];

  for (i = 0; i < NUM_LEDS; i++) {
		next_pwm_vals[i] = pwm_value;
  }
}

/*
long sawtooth_time_index = 0;
const long sawtooth_offsets[NUM_LEDS] = {0, MAX_CIE_INDEX/3, (2*MAX_CIE_INDEX)/3};
void Sawtooth() {
	// Just change value once per flash
	sawtooth_time_index = (sawtooth_time_index + 1) % MAX_CIE_INDEX;

	for (i = 0; i < NUM_LEDS; i++) {
		next_pwm_vals[i] = CIE[(sawtooth_time_index + sawtooth_offsets[i]) % MAX_CIE_INDEX];
  }
}
*/
void setup_deep_sleep()
{

	// enable power interface module clock
	RCC->APB1PCENR |= RCC_APB1Periph_PWR;

	// enable low speed oscillator (LSI)
	RCC->RSTSCKR |= RCC_LSION;
	while ((RCC->RSTSCKR & RCC_LSIRDY) == 0) {}

	// enable AutoWakeUp event
	EXTI->EVENR |= EXTI_Line9;
	EXTI->FTENR |= EXTI_Line9;

	// configure AWU prescaler
	PWR->AWUPSC |= PWR_AWU_Prescaler_1024;

	// configure AWU window comparison value
	PWR->AWUWR &= ~0x3f;
	PWR->AWUWR |= 1;

	// enable AWU
	PWR->AWUCSR |= (1 << 1);

	// select standby on power-down
	PWR->CTLR |= PWR_CTLR_PDDS;

	// peripheral interrupt controller send to deep sleep
	PFIC->SCTLR |= (1 << 2);
}


/*
 * initialize TIM1 for PWM
 */
void aemhead_init( void )
{
	// Enable GPIOC, GPIOD and TIM1
	RCC->APB2PCENR |= 	RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA |
						RCC_APB2Periph_TIM1;


	// Set up defaults of all pins for lowest power standby
	GPIOA->CFGLR = (GPIO_CNF_IN_PUPD<<(4*2)) |
				   (GPIO_CNF_IN_PUPD<<(4*1));
	GPIOA->BSHR = GPIO_BSHR_BS2 | GPIO_BSHR_BR1;
	GPIOC->CFGLR = (GPIO_CNF_IN_PUPD<<(4*7)) |
				   (GPIO_CNF_IN_PUPD<<(4*6)) |
				   (GPIO_CNF_IN_PUPD<<(4*5)) |
				   (GPIO_CNF_IN_PUPD<<(4*4)) |
				   (GPIO_CNF_IN_PUPD<<(4*3)) |
				   (GPIO_CNF_IN_PUPD<<(4*2)) |
				   (GPIO_CNF_IN_PUPD<<(4*1)) |
				   (GPIO_CNF_IN_PUPD<<(4*0));
	GPIOC->BSHR = GPIO_BSHR_BS7 |
				  GPIO_BSHR_BS6 |
				  GPIO_BSHR_BS5 |
				  GPIO_BSHR_BS4 |
				  GPIO_BSHR_BS3 |
				  GPIO_BSHR_BS2 |
				  GPIO_BSHR_BS1 |
				  GPIO_BSHR_BS0;
	GPIOD->CFGLR = (GPIO_CNF_IN_PUPD<<(4*7)) |
				   (GPIO_CNF_IN_PUPD<<(4*6)) |
				   (GPIO_CNF_IN_PUPD<<(4*5)) |
				   (GPIO_CNF_IN_PUPD<<(4*4)) |
				   (GPIO_CNF_IN_PUPD<<(4*3)) |
				   (GPIO_CNF_IN_PUPD<<(4*2)) |
				   (GPIO_CNF_IN_PUPD<<(4*0));
	GPIOD->BSHR = GPIO_BSHR_BS7 |
				  GPIO_BSHR_BS6 |
				  GPIO_BSHR_BS5 |
				  GPIO_BSHR_BS4 |
				  GPIO_BSHR_BS3 |
				  GPIO_BSHR_BS2 |
				  GPIO_BSHR_BS0;




	// PD0 is BUTTON_STYLE, Input with pull-up/pull-down resistors
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_PUPD)<<(4*0);
	// Set up the pin as pull down
	GPIO_digitalWrite_lo(GPIOv_from_PORT_PIN(GPIO_port_D, 0));

	// PD2 is T1CH1, 2MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf<<(4*2));
	GPIOD->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP_AF)<<(4*2);

	// PC3 is T1CH3, 2MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*3));
	GPIOC->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP_AF)<<(4*3);

	// PC4 is T1CH4, 2MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);


	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

	// Prescaler
	TIM1->PSC = 0x4;

	// Auto Reload - sets period
	TIM1->ATRLR = MAX_PWM_VAL-1; // So off is actually off apparently

	// One pulse mode (pulse and then stop)
	TIM1->CTLR1 |= TIM_OPM;

	// Enable CH1 output, positive pol
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;

	// Enable CH2 output, positive pol
	TIM1->CCER |= TIM_CC2E | TIM_CC2P;

	// Enable CH3 output, positive pol
	TIM1->CCER |= TIM_CC3E | TIM_CC3P;

	// Enable CH4 output, positive pol
	TIM1->CCER |= TIM_CC4E | TIM_CC4P;

	// CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 011)
	TIM1->CHCTLR1 |= TIM_OC1M_0 | TIM_OC1M_1;
	TIM1->CHCTLR1 |= TIM_OC2M_0 | TIM_OC2M_1;

	// CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 011)
	TIM1->CHCTLR2 |= TIM_OC3M_0 | TIM_OC3M_1;
	TIM1->CHCTLR2 |= TIM_OC4M_0 | TIM_OC4M_1;

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

typedef void (*style)(void);
style styles[] = {&LEDBeats, &Breathe};
uint8_t style_index = 0;

void increment_style_index() {
	style_index = (style_index + 1) % (sizeof(styles) / sizeof(styles[0]));
}

void write_pwm_vals() {
	for (i = 0; i < NUM_LEDS; i++) {
		t1pwm_setpw(i, next_pwm_vals[i]);
	}
}

// Intent is to have millis() == 0 after this function runs
void reset_millis_offset() {
	millis_start = SysTick->CNT / DELAY_MS_TIME;
	deep_sleep_time_ms = 0;
}

void update_button_state() {
	// Check style button state
	// If user pressed the button, they must release it before pressing again
	// (time held doesn't matter)
	if (button_is_pressed == 0 &&
			GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_D, 0)) == high &&
			millis() > (button_released_press_ms + BUTTON_DEBOUNCE_DELAY_MS))
	{
		button_is_pressed = 1;
		increment_style_index();
		// Start patterns at the beginning
		reset_millis_offset();
		button_started_press_ms = millis();
	}
	if (button_is_pressed == 1 &&
			GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_D, 0)) == low &&
			millis() > (button_started_press_ms + BUTTON_DEBOUNCE_DELAY_MS))
	{
		button_is_pressed = 0;
		button_released_press_ms = millis();
	}
}

int main()
{
	// For now, run ../ch32v003fun/minichlink/minichlink -u to unbrick and wipe the flash
	// so you can repogramH
	// TODO: Would like to move to just holding down the style button to enter bootloader mode
	//       when powering on but didn't get there in time

	SystemInit();

	// init TIM1 for PWM
	aemhead_init();

	//RCC->CFGR0 = BASE_CFGR0_NEW;


  reset_millis_offset();
#ifdef DEEP_SLEEP
	setup_deep_sleep();
#endif
	while(1) {
		update_button_state();

		// Write pwm values into timer registers
		write_pwm_vals();

		// Enable timer1 (one-shot)
		TIM1->CTLR1 |= TIM_CEN;

		// Determine next values of LEDs for next awake period while
		// timer is running
		styles[style_index]();

		// Wait until TIM1 is done with pulse
		while (TIM1->CTLR1 & TIM_CEN);

#ifdef DEEP_SLEEP
		// Go to sleep
		__WFE();
		// Restore clocks, etc
		SystemInit();
		//
		deep_sleep_time_ms += DEEP_SLEEP_TIME_MS;
#else
		Delay_Ms( DEEP_SLEEP_TIME_MS );
#endif
	}
}

// TODO before ship:
//  * Clean up unused code for clarity
//  * Decide tabs/spaces

//  Patterns:
//   * Sawtooth like Alton
//   * Something random but not taxing

// Nice to have
//  * Adjust brightness by holding down button. Adjusts prescalar from a few set values in a circular array. Set PWM brightness to max during this time?
//  * Save settings...https://github.com/cnlohr/ch32v003fun/pull/85 and https://github.com/recallmenot/ch32v003fun_wildwest/blob/main/lib%2Fch32v003_flash.h