/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
#include <stdio.h>

#define MAX_PWM_VAL 1024

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
	TIM1->ATRLR = MAX_PWM_VAL; //65535;//255;
	
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
 * set timer channel PW
 */
void t1pwm_setpw(uint8_t chl, uint16_t width)
{
	switch(chl&3)
	{
		case 0: TIM1->CH1CVR = width; break;
		case 1: TIM1->CH2CVR = width; break;
		case 2: TIM1->CH3CVR = width; break;
		case 3: TIM1->CH4CVR = width; break;
	}
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



// A bit hacky, but works for my purposes for now
#include "patterns.c"

//#include "stm32f0xx.h"
/*
 * entry
 */
int main()
{
	uint16_t pwm = 0;
	
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
		
		
		pwm = (pwm + 2) % MAX_PWM_VAL;
		t1pwm_setpw(1, MAX_PWM_VAL-pwm); // Chl 1
		TIM1->CTLR1 |= TIM_CEN;

		// Determine next values of LEDs. 
		// Theoretically only MAX_PWM_VAL ticks of CPU available, but can 
		// increase cpu freq if needed
		//LEDBeats();
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