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


int val=0; // to store the ADC value
void ADC_init() //using ADC1 and GPIOA Pin 5 (PA5)
{
	//Configuring ADC1 and using the ADC5 i.e. PA5 pin of STM32 for analog reading
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	// enabling the clock to the ADC peripheral
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // enabling the alternate function
	GPIOA->CRL &= ~(GPIO_CRL_CNF5_1); 	//setting GPIO configuration as analog input mode
	GPIOA->CRL &= ~(GPIO_CRL_CNF5_0);   //setting GPIO configuration as analog input mode
	// The mode of all pins is by default set as input, no bits to be set in the MODE bits

	ADC1->CR2 &= ~(ADC_CR2_ADON); //disabling the ADC prior to any configurations
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV4 ; // dividing the bus clock by 4. So we get 8Mhz for 32 Mhz bus clock
	ADC1->SMPR2 |= ADC_SMPR2_SMP5_0; // setting the max sample time for better readings
	ADC1->SMPR2 |= ADC_SMPR2_SMP5_1; // setting the max sample time for better readings
	ADC1->SMPR2 |= ADC_SMPR2_SMP5_2; // setting the max sample time for better readings
	ADC1->SQR3 |= ADC_SQR3_SQ1_0; 	//setting up channel 5 as first sequence
	ADC1->SQR3 |= ADC_SQR3_SQ1_2; 	//setting up channel 5 as first sequence
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1_1);//setting up channel 5 as first sequence
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1_3);//setting up channel 5 as first sequence
	ADC1->CR2 |= ADC_CR2_CONT; //enabling the continuous mode
	ADC1->CR2 |= ADC_CR2_ADON; //enabling the ADC
	delay_ms(1); // slight delay
	ADC1->CR2 |= ADC_CR2_ADON; //enabling the ADC for 2nd time to actually enable ADC
	ADC1->CR2 |= ADC_CR2_CAL; // enable the calibration. The bit turns 0 when calibration is completed
	while(ADC1->CR2 & ADC_CR2_CAL); // wait until the calibration is completed.
	delay_ms(1); // slight delay
}
int ADC_Read()
{
	while(!(ADC1->SR & ADC_SR_EOC));
	return ADC1->DR;
}

int main(void)
{
system_clk();
timer2_init();
ADC_init();

RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // enabling the clock source for GPIOC register.
GPIOC->CRH |= (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0); // enable max speed to 50Mhz.
GPIOC->CRH &= ~(GPIO_CRH_CNF13_1 | GPIO_CRH_CNF13_0 );


	while(1)
	{

		val= ADC_Read();
		GPIOC->ODR |= GPIO_ODR_ODR13;  	// using the Output data register(ODR) to set the pin high
		delay_ms(val/2);
		GPIOC->ODR &= ~(GPIO_ODR_ODR13);// using the Output data register(ODR) to set the pin low
		delay_ms(val/2);
	}
return 0;
}
