#include <stdint.h>
#include<stm32f1xx.h>

//Example of creating a function that generates PWM signal based on Timer and Output Compare registers
//We will be generating a PWM signal for Timer 2 channel 1. Which corresponds to PA0 pin of STM32 on bluepill
// The Timer ARR register is used to determine the period of the PWM signal
// The capture capture compare register(CCR)register. Here CCR2 for Timer 2 determines the duty cycle

//The frequency of the PWM is determined by the TIMx_ARR - auto reload register
//The duty cycle of the PWM is determined by the CCRx register- Capture compare register where x= channel
//PWM mode can be selected by setting the OCM bits of the TIMx_CCMRx register
//CCMR1 deals with channel 1 and 2 of a timer while CCMR2 deals with channel 3  and 4

/*TIMx_CCER register is used to enable the Capture compare on the 4 channels

Preload register is a register to which the value is written and stored in a shadow if it is enabled.
In hardware, it is going to preload the value that is given as input to a shadow register and the CCRx
register is only updated after an over flow event
So if preload is enabled, the value of user input is store in shadow register then given to CCRx after overflow event
If prelod is disabled, the value is immediately stored in CCRx register
This is controlled by OCxPE bit in CCMRx register corresponding to the timer we are using. x= 1,2,3,4

Next, we have to set the UG (update generation) bit in TIMx_EGR register (event generation register)
*/

//Summary of setting up the PWM is as follows:
/*
1. Enable the clock to the GPIO using the APB1 or APB2 register
Next, The alternate function mode has to be enabled so that the pin will be linked to the timer
2. Configure the Pin as alternate function push pull pin and set the mode as output mode at any speed you wish
3. (Optional) Enable the preload register for output compare CCR and auto reload register
4. Enable the capture compare mode in the TIMx_CCER ( Timerx capture compare enable register) for the particular channel of the timer
5. Set the PWM mode using the OCM bits of the CCMRx register. x=1,2,3..
6. Set the Timer Prescaler and AutoReload Register
7. Set the CCR register value
8. Set the update Generation bit in the TIMx_EGR register
9. Enable the timer using the CEN bit of the control register.
*/

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
    /* Loop forever */
	for(;;);
}
