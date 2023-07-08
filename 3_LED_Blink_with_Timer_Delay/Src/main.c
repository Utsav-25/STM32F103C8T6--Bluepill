#include <stdint.h>
#include<stm32f1xx.h>

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

void timer2_init() // creating a function to initialize the timer.Check Project 2 for details
{

	TIM2->CR1 &= ~(TIM_CR1_CEN);
	TIM2->SR &= ~(TIM_SR_UIF);
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->EGR |= TIM_EGR_UG;
	TIM2->PSC = 32;
	TIM2->ARR = 0xffff;
	TIM2->CR1 |= TIM_CR1_CEN; // enable the timer
}

void delay_us(uint16_t us)
{
	TIM2->CNT=0;
	while(TIM2->CNT< us);
}
void delay_ms(uint16_t ms)
{
	for(uint16_t i=0;i<ms;i++)
	delay_us(1000);
}
void delay_sec(uint16_t sec)
{
	for(uint16_t i=0;i<sec;i++)
	delay_ms(1000);
}

int main(void)
{
	system_clk();
	timer2_init();
	//1. Enable the clock for the given GPIO peripheral.
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // enabling the clock source for GPIOC register.
	//2. Configure the GPIO mode and Max speed
	GPIOC->CRH |= (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0); // enable max speed to 50Mhz.
	GPIOC->CRH &= ~(GPIO_CRH_CNF13_1 | GPIO_CRH_CNF13_0 ); //general purpose Push Pull.

	while(1)
	{
		GPIOC->ODR ^= GPIO_ODR_ODR13;
		delay_sec(1);
	}
}
