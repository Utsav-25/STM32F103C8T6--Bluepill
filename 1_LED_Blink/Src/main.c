#include <stdint.h>
#include<stm32f1xx.h>

//STM32 Clock Configuration.
//For configuring the clock, we need to do the following steps in sequential manner.
//1. Enable the HSE(High speed External source, usually Xtal) and wait for it to become stable
//2. Configure the flash prefetch and latency settings
//3. Configure the clock for busses (AHB, APB1, APB2)
//4. Configure the PLL multiplier/ HSE divider
//5. Enable the PLL source and wait for it to become stable
//6. Enable the clock source and wait for it to set

void system_clk() // creating a function to configure STM32 clock
{
//1. Enable the HSE and wait for it to become stable
	RCC->CR |= RCC_CR_HSEON; //HSE is 8Mhz in BluePill.
	//waiting for it to be stable
	while(!(RCC->CR & RCC_CR_HSERDY));
//2. Configure the flash prefetch and latency settings
	FLASH->ACR |= FLASH_ACR_PRFTBE; //enable the prefetch buffer
	FLASH->ACR |= FLASH_ACR_LATENCY_1; // one wait state as system clock will be 32 Mhz.
//3. Configure the PLL clock
	RCC->CFGR |= RCC_CFGR_PLLSRC;
	//HSE CLOCK DIVIDER ( Here we have not divided the HSE clock)
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE;
//4. PLL Multiplier, we are multiplying HSE by 4.
	RCC->CFGR |= RCC_CFGR_PLLMULL4;
//5. Configure the clock for busses APB1, APB2 and AHB
	RCC->CFGR |= (RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_HPRE_DIV1);
//6. Enable the PLL
	RCC->CR |= RCC_CR_PLLON;
	//Wait for PLL to be stable
	while(!(RCC->CR & RCC_CR_PLLRDY));
//7. Enable the system clock and wait for it to be stable
	RCC->CFGR |= RCC_CFGR_SW_PLL; // choosing PLL as clock source
	while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
}

int main(void)
{
	/* Code for the LED Blink Begins*/
	system_clk(); // initializing the system clock.
//1. Enable the clock for the given GPIO peripheral.
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // enabling the clock source for GPIOC register.
//2. Configure the GPIO mode and Max speed
	GPIOC->CRH |= (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0); // enable max speed to 50Mhz.
	GPIOC->CRH &= ~(GPIO_CRH_CNF13_1 | GPIO_CRH_CNF13_0 ); //general purpose Push Pull.
    /* Loop forever */
	while(1)
	{
		GPIOC->ODR |= GPIO_ODR_ODR13;  	// using the Output data register(ODR) to set the pin high
		for(int i=0;i<2000000;i++){} 	// random delay
		GPIOC->ODR &= ~(GPIO_ODR_ODR13);// using the Output data register(ODR) to set the pin low
		for(int i=0;i<2000000;i++){}	// random delay
	}
}
