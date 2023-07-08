#include <stdint.h>
#include "stm32f1xx.h"

/*
In this example we would be looking at the Analog to Digital converter
peripheral for reading analog values of a single channel in continuous mode

We will be using channel 5 of the ADC1 which is the GPIOA pin 5 or PA5

Few points to note:
1.The ADC input clock is generated from the PCLK2 clock divided by a prescaler
and it must not exceed 14 MHz

The overall steps to configure and enable the ADC are:

1. Enable the clock to the ADC peripheral and Alternate Function peripheral and GPIO peripheral
2. Setup the GPIO as alternate function push pull and in input mode
3. Changing the prescaler of ADC so as to not exceed 14Mhz.
	Our bus clock is running at 32MHz and hence we need to set the
	ADCPRE bits in RCC CFGR register so that the clock is divided by 4
4. Setting up the sampling rate in the ADC sample time register. This configures the number of samples
   the ADC takes. More sample time gives better readings but also requires more clock cycles
5. Setting up the sequence register. It determines how many ADC channels are to be sequenced and in what order
	Since we are using only one channel here,
		1. the default value for channel sequence length is for 1 channel
		2. In the first sequence, we need to put the channel 5
6. Set the ADC in continuous mode and enable the ADC TWICE to actually turn it on
	We have to enable the ADC twice in order to actually turn it on with a slight
	delay between each event. For the first time it wakes up from the power down mode,
	when ADON is initiated 2nd time, that is when the conversion begins
7. Enable the Calibration the ADC and wait for it to complete

We then define a function to read the value of data register after end of conversion
*/
int val=0; // variable to store the ADC value
void ADC_init()
{
	//1. Enabling the clock to ADC1 peripheral and Alternate function peripheral
		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	// enabling the clock to the ADC peripheral
		RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // enabling the alternate function
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	// enabling the clock for GPIOA

	//2. Configuring the GOIO as alternate function and input mode (Pin PA5 for ADC5)
		GPIOA->CRL &= ~(GPIO_CRL_CNF5_1); 	//setting GPIO configuration as analog input mode
		GPIOA->CRL &= ~(GPIO_CRL_CNF5_0);   //setting GPIO configuration as analog input mode
	// The mode of all pins is by default set as input, no bits to be set in the MODE bits

		ADC1->CR2 &= ~(ADC_CR2_ADON); //disabling the ADC prior to any configurations

	//3. Configuring clock prescaler for ADC so as to not exceed 14Mhz
		RCC->CFGR |= RCC_CFGR_ADCPRE_DIV4 ; // dividing the bus clock by 4. So we get 8Mhz for 32 Mhz bus clock

	//4. Setting up the sample time register
		ADC1->SMPR2 |= ADC_SMPR2_SMP5_0; // setting the max sample time for better readings
		ADC1->SMPR2 |= ADC_SMPR2_SMP5_1; // setting the max sample time for better readings
		ADC1->SMPR2 |= ADC_SMPR2_SMP5_2; // setting the max sample time for better readings

	//5. Setting up the channel in the sequence register
		//Here we have to set channel 5 as 1st sequence in the ADC1 SQR3 register
		//ADC_SQR3-> SQ1 = 0101 for channel 5
		ADC1->SQR3 |= ADC_SQR3_SQ1_0; 	//setting up channel 5 as first sequence
		ADC1->SQR3 |= ADC_SQR3_SQ1_2; 	//setting up channel 5 as first sequence
		ADC1->SQR3 &= ~(ADC_SQR3_SQ1_1);//setting up channel 5 as first sequence
		ADC1->SQR3 &= ~(ADC_SQR3_SQ1_3);//setting up channel 5 as first sequence

	//6. Set the continuous mode for the ADC and enable the ADC twice for starting it
		ADC1->CR2 |= ADC_CR2_CONT; //enabling the continuous mode
		ADC1->CR2 |= ADC_CR2_ADON; //enabling the ADC
		for(int i=0;i<100000;i++); // slight delay .This can be later replaced by a proper delay routine
		ADC1->CR2 |= ADC_CR2_ADON; //enabling the ADC for 2nd time to actually enable ADC
		for(int i=0;i<100000;i++); // slight delay. This can be later replaced by a proper delay routine

	//7. Enable the calibration of ADC and wait for it to complete
		ADC1->CR2 |= ADC_CR2_CAL; // enable the calibration. The bit turns 0 when calibration is completed
		while(ADC1->CR2 & ADC_CR2_CAL); // wait until the calibration is completed.
		for(int i=0;i<100000;i++); // slight delay. This can be later replaced by a proper delay routine
}

int ADC_Read() //defining a function to return ADC data register value once conversion is complete.
{
	while(!(ADC1->SR & ADC_SR_EOC));
	return ADC1->DR;
}


int main(void)
{
	while(1)
	{
		val=ADC_Read();
	}
}
