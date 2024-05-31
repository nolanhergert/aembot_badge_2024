/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
#include <stdio.h>

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



	// PC0 is T1CH3 alternate mapping, 10MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*0);
	//AFIO->PCFR1 |= GPIO_PartialRemap1_TIM1;


		
	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
	
	// CTLR1: default is up, events generated, edge align
	// SMCFGR: default clk input is CK_INT
	
	// Prescaler 
	TIM1->PSC = 0x00;
	
	// Auto Reload - sets period
	//TIM1->ATRLR = 65535;//255;
	
	// Reload immediately
	TIM1->SWEVGR |= TIM_UG;
	
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
	
	// Set the Capture Compare Register value to 50% initially
	TIM1->CH1CVR = 48;
	TIM1->CH2CVR = 48;
	TIM1->CH3CVR = 48;
	TIM1->CH4CVR = 48;
	
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
 * force output (used for testing / debug)
 */
void t1pwm_force(uint8_t chl, uint8_t val)
{
	uint16_t temp;
	
	chl &= 3;
	
	if(chl < 2)
	{
		temp = TIM1->CHCTLR1;
		temp &= ~(TIM_OC1M<<(8*chl));
		temp |= (TIM_OC1M_2 | (val?TIM_OC1M_0:0))<<(8*chl);
		TIM1->CHCTLR1 = temp;
	}
	else
	{
		chl &= 1;
		temp = TIM1->CHCTLR2;
		temp &= ~(TIM_OC1M<<(8*chl));
		temp |= (TIM_OC1M_2 | (val?TIM_OC1M_0:0))<<(8*chl);
		TIM1->CHCTLR2 = temp;
	}
}

// A bit hacky, but works for my purposes for now
#include "patterns.c"

//#include "stm32f0xx.h"
/*
 * entry
 */
int main()
{
	uint16_t count = 15;
	
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
		t1pwm_setpw(0, 65535-count); // Chl 1
		Delay_Ms( 10 );
		count = (count + 1) % 1024;
	}

	
}
