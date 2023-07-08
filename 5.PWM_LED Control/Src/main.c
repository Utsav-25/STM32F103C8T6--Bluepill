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

//Creating a function to make pin PA0 as output.
void T2C1_GPIO_Config()
{
	//1. Enabling the clock signal to GPIOA and alternate function IO
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // Clock enabled to the GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // alternate function IO clock enabled

	//2. Configure the GPIO in alternate function mode and at Max speed
	GPIOA->CRL |= ( GPIO_CRL_MODE0_0 | GPIO_CRL_MODE0_1) ;// enable max speed to 50Mhz.
	GPIOA->CRL |= GPIO_CRL_CNF0_1;  	//alternate function Push Pull
	GPIOA->CRL &= ~(GPIO_CRL_CNF0_0);	//alternate function Push Pull
}

void pwm_init()
{
	//Make sure that the timer is disabled.
	TIM2->CR1 &= ~(TIM_CR1_CEN); // setting the counter enable bit to 0 in the TIMx Control Register 1
	TIM2->SR &= ~(TIM_SR_UIF); 	 // setting the update interrupt flag to 0. This bit is set by hardware when the registers are updated.We have to make sure to reset it by software.
	//Enable the T2C1 GPIO pin:
	T2C1_GPIO_Config();
	//Enable the Timer 2 Peripheral clock:
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	//3. (Optional) Enable the Preload register for Output compare and ARR.
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;
	//Enable the Preload register for ARR register
	TIM2->CR1 |= TIM_CR1_ARPE;
	// we need to disable any kind of update generation before setting our timer prescaler or ARR values
	//this is done by setting the UDIS bit to high in the TIMx CR1 register.
	TIM2->CR1 |= TIM_CR1_UDIS; // by setting this bit high, no update event is generated.

	//4. Enable the capture compare mode
	TIM2->CCER |= TIM_CCER_CC1E; // enable the capture compare on channel 1.

	//5. Set the output compare mode as PWM mode by setting the 3 bits of OC1M to 110
	TIM2->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
	TIM2->CCMR1 &=~(TIM_CCMR1_OC1M_0);

	//6. Set the timer prescaler and autoreload register
	TIM2->PSC =32; // divided the input clock frequency by 32
	TIM2->ARR = 20000; // determines upto what value it will count

	// disable the UDIS bit so that update event would not be generated.
	TIM2->CR1 &= ~(TIM_CR1_UDIS); // by setting this bit low,update event is generated.

	//7. Set the CCR value
	TIM2->CCR1 = 200; // determines the duty cycle

	//Set the output signal polarity of channel 1 as active high
	TIM2->CCER &= ~(TIM_CCER_CC1P);

	//8. Set the Update generation bit in EGR register and clear the Update interrupt flag
	TIM2->EGR |= TIM_EGR_UG; // enable the event generation for shadow register
	TIM2->SR &= ~(TIM_SR_UIF); // clear the Update interrupt flag that has been set high by UG bit in case of evenet generation.

	//9. Enable the timer . This should be done at the last after setting all the configurations
	TIM2->CR1 |= TIM_CR1_CEN; // enable the timer

}

int main(void)
{
unsigned long int i;
unsigned long int j;

	system_clk();
	pwm_init();

	while(1)
	{
		for(i=0; i<20000; i++)
		{
		TIM2->CCR1 = i;
		for (j=0;j<200; j++){}
		}

		for(i=20000; i>0; i--)
		{
		TIM2->CCR1 = i;
		for (j=0;j<200; j++){}
		}
		for (j=0;j<10000; j++){}
	}
}
