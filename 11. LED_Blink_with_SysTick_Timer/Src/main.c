#include <stdint.h>
#include "stm32f1xx.h"

void system_clk() // creating a function to configure STM32 clock.Refer project 1 for details
{

	RCC->CR |= RCC_CR_HSEON; //HSE is 8Mhz in BluePill.
	while(!(RCC->CR & RCC_CR_HSERDY));
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	FLASH->ACR |= FLASH_ACR_LATENCY_1;
	RCC->CFGR |= RCC_CFGR_PLLSRC;
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE;
	RCC->CFGR |= RCC_CFGR_PLLMULL4; // 8MHz * 4= 32Mhz of system clock
	RCC->CFGR |= (RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_HPRE_DIV1);
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY));
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
}
/*
In this program we will look at the SysTick timer of the STM32 micro controller.
It is directly attached to the CPU core (ARM cortex M4 in this case) and provided clock signals
The systick timer can be used to generate delays but only in Milliseconds.
Can be useful as we are not deploying any timers for simple delay applications

These are the following steps to configure and use SysTick for delay genration:

1. Configure the systick control register to 0 so as to reset all initial settings
2. We then load the systick load register with a value, this is the highest value
   till which the systick will count upwards. It is a 24 bit value, so max number = 0x00FFFFFF
3. Initializing the systick value as 0 during configuration.
4. Select the clock source for the SysTick as AHB. For our case the AHB is running at 32Mhz

In the delay function,
5. Put the counter value in LOAD register, based on AHB clock and the amount of delay we want
6. Load the VAL as 0
7. Enable the systick timer
8. Wait until the counter flag is enabled.

*/

void SysTick_init()
{
	SysTick->CTRL=0; 			// initializing the systick control register with all reset values
	SysTick->LOAD=0x00FFFFFF; 	// putting the max value in systick load register
	SysTick->VAL=0; 			//setting 0 value initially
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk; // setting AHB clock as systick clock source
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // enable the systick timer
}

void delay_ms(uint16_t x)
{	// as our clock is running at 32MHz and we need 1 Millisecond delay, we need to put a
	// value 1000 times smaller than the AHB clock value in the SysTick load register.
	// Since it counts till 0, the LOAD value will be 31999 or (32000-1)
	//Here x represents the time in milliseconds for which we want the delay
	for(;x>0;x--)
	{
	SysTick->LOAD= (32000-1);
	SysTick->VAL=0;	// loading the current value as 0
	while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)==0);
	}
}

int main(void)
{
	system_clk();
	SysTick_init();

		//1. Enable the clock for the given GPIO peripheral.
		RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // enabling the clock source for GPIOC register.
		//2. Configure the GPIO mode and Max speed
		GPIOC->CRH |= (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0); // enable max speed to 50Mhz.
		GPIOC->CRH &= ~(GPIO_CRH_CNF13_1 | GPIO_CRH_CNF13_0 ); //general purpose Push Pull.
	while(1)
	{
		GPIOC->ODR ^= GPIO_ODR_ODR13;
				delay_ms(1000);
	}
}

