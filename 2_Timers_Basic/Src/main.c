#include <stdint.h>
#include<stm32f1xx.h>
//for STM32F103C8T6 there are 3 general purpose timers TIME2, TIME3 and TIME4
//there is one advanced timer TIME1 . The details can be found from data sheet and reference manual

//There are 4 main registers that needs to be taken care while initializing a timer
/*
1. Counter register
	A 16 bit register that increments its value with every clock tick of the source clock
	Counting range is from 0 to 65535 (2^16-1)
2. PreScale register
	The clock source to the timer or the APB1 or APB2 register is divided by a certain prescale factor
3. AutoReload register
	AutoReload resister contains the count till which the counter register will count before resetting
4. Repetition counter register.
*/

/*Timer frequency = bus clock (System clock)/( prescaler +1)
 Time delay by timer= Timer time period * Autoreload resigter value (TIMxARR , where x= timer number).
 For system clock frequency of 32 Mhz and prescale value of 31,
 Timer frequency = 32Mhz/(31+1) = 1 MHz. So Timer Time Period (TTP) = 1 Microsecond
 */

void timer_init() // creating a function to initialize the timer.
{
// WE Will be using Timer 2 for our example.

	//1. Make sure that the timer is disabled.
	TIM2->CR1 &= ~(TIM_CR1_CEN); // setting the counter enable bit to 0 in the TIMx Control Register 1
	TIM2->SR &= ~(TIM_SR_UIF); 	 // setting the update interrupt flag to 0. This bit is set by hardware when the registers are updated.We have to make sure to reset it by software.
	//2. Enable the TIMER2 peripheral clock.
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	//3. Enable the Update Generation in Event Generation Register
	// This bit is used to re-inititalize the counter and generate an update of the registers
	// The update generation bit needs to be set before any prescaler settings because this bit
	// also clears the preScaler counter
	TIM2->EGR |= TIM_EGR_UG; // enabling update generation.

	//4. Setting up the timer prescaler value ( to 32 in our case).
	TIM2->PSC = 32;
	//5. Setting up the AutoReload Register value:
	TIM2->ARR = 0xffff; // 65535 in decimal.
	//6. Enable the timer
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
    /* Loop forever */
	for(;;);
}
