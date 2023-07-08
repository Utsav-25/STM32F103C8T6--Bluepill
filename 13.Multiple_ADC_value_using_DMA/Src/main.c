#include <stdint.h>
#include "stm32f1xx.h"
/*
 In this example we will access multiple ADC values based on DMA(Direct memory access)
 For this we have to initialize the ADC and the sequence registers in which we will define
 how many pins and in what sequence will the data be stored.

For this we also need to specify the following in the ADC initialization
	i.	sequence length in the ADC1 SQR1 register. This is the number of analog readings, 2 in out case
	ii. Enable the scan mode in the ADC CR1 register to scan the ADC inputs of the sequence
	iii.Enable the DMA mode in the ADC CR2 register

 The main steps to access and store  ADC values of multiple channels using DMA are:
!Disable the DMA before configuring
1. Enable the clock to the DMA peripheral(DMA1 or DMA2) in the RCC control register
2. Specify the peripheral address in the DMA CPAR register, this is the register address from which DMA will take the values
3. Specify the memory address where the data will be stored in the DMA CMAR register
4. Specify the number of data to be transferred in the DMA CNDTR register , 2 in our case
5. Enable the Circular mode. This means the DMA is going to iterate through the memory array,
	once it reaches the end, it again starts from the first memory location
6. Enable the Memory increment mode. This means that after writing to a memory location the memory address is
	incremented to the next one where the other data is stored
7. Specify the peripheral register size (in our case ADC DR register is of 16 bit)
8. Specify the memory size of DMA (16 bit in our case as ADC DR is of 16 bit)
!Enable the DMA after configuring
*/

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
void SysTick_init()// using Systick timer to create delays in order of milliseconds
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

//initializing the ADC_init function with sequence length of 2 conversions
void ADC_init() //using ADC1 and GPIOA Pin 5(PA5) and GPIOA Pin 6(PA6)
{
	//Configuring ADC1 and using the ADC5 and ADC6 i.e. PA5 and PA6 pin of STM32 for analog reading
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	// enable the clock for GPIOA peripheral
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	// enabling the clock to the ADC peripheral
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // enabling the alternate function
	GPIOA->CRL &= ~(GPIO_CRL_CNF5_1); 	//setting GPIOA pin 5 configuration as analog input mode
	GPIOA->CRL &= ~(GPIO_CRL_CNF5_0);   //setting GPIOA pin 5 configuration as analog input mode
	GPIOA->CRL &= ~(GPIO_CRL_CNF6_1); 	//setting GPIOA pin 6 configuration as analog input mode
	GPIOA->CRL &= ~(GPIO_CRL_CNF6_0);   //setting GPIOA pin 6 configuration as analog input mode
	// The mode of all pins is by default set as input, no bits to be set in the MODE bits

	ADC1->CR2 &= ~(ADC_CR2_ADON); //disabling the ADC prior to any configurations
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV4 ; // dividing the bus clock by 4. So we get 8Mhz for 32 Mhz bus clock

	ADC1->SMPR2 |= ADC_SMPR2_SMP5_0 | ADC_SMPR2_SMP5_1 | ADC_SMPR2_SMP5_2; // setting the max sample time for better readings
	ADC1->SMPR2 |= ADC_SMPR2_SMP6_0 | ADC_SMPR2_SMP6_1 | ADC_SMPR2_SMP6_2; // setting the max sample time for better readings

	ADC1->SQR1|=  ADC_SQR1_L_0;  // setting the sequence length for 2 conversions
	ADC1->SQR1&=~(ADC_SQR1_L_1); // setting the sequence length for 2 conversions
	ADC1->SQR1&=~(ADC_SQR1_L_2); // setting the sequence length for 2 conversions
	ADC1->SQR1&=~(ADC_SQR1_L_3); // setting the sequence length for 2 conversions

	ADC1->SQR3 |= ADC_SQR3_SQ1_0; 	//setting up channel 5 as first sequence
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1_1);//setting up channel 5 as first sequence
	ADC1->SQR3 |= ADC_SQR3_SQ1_2; 	//setting up channel 5 as first sequence
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1_3);//setting up channel 5 as first sequence

	ADC1->SQR3 &= ~(ADC_SQR3_SQ2_0);//setting up channel 6 as second sequence
	ADC1->SQR3 |= ADC_SQR3_SQ2_1;	//setting up channel 6 as second sequence
	ADC1->SQR3 |= ADC_SQR3_SQ2_2; 	//setting up channel 6 as second sequence
	ADC1->SQR3 &= ~(ADC_SQR3_SQ2_3);//setting up channel 6 as second sequence

	ADC1->CR2 |= ADC_CR2_CONT; //enabling the continuous mode
	ADC1->CR1 |= ADC_CR1_SCAN; //enabling the scan mode in case of multiple conversions
	ADC1->CR2 |= ADC_CR2_DMA;  //enable the DMA access to this peripheral
	ADC1->CR2 |= ADC_CR2_ADON; //enabling the ADC
	delay_ms(1); // slight delay
	ADC1->CR2 |= ADC_CR2_ADON; //enabling the ADC for 2nd time to actually enable ADC
	ADC1->CR2 |= ADC_CR2_CAL; // enable the calibration. The bit turns 0 when calibration is completed
	while(ADC1->CR2 & ADC_CR2_CAL); // wait until the calibration is completed.
	delay_ms(1); // slight delay
}

uint16_t ADCsamples[2]={0,0};

void DMA_Config() // initializing the DMA to access and store 2 ADC values from PA5 and PA6
{
	DMA1_Channel1->CCR &=~(DMA_CCR_EN); // disable the DMA prior to any settings

	RCC->AHBENR|= RCC_AHBENR_DMA1EN; // enabling clock to DMA1.
	DMA1_Channel1->CPAR = (uint32_t)&(ADC1->DR); // specifying the address of the Data Register of ADC1
	DMA1_Channel1->CMAR = (uint32_t)ADCsamples; // specifying the memory address where data will be stored
	DMA1_Channel1->CNDTR=2; // number of data to be transferred
	DMA1_Channel1->CCR|= DMA_CCR_CIRC; //enable the circular mode
	DMA1_Channel1->CCR|= DMA_CCR_MINC; //enable the memory increment mode
	DMA1_Channel1->CCR|= DMA_CCR_PSIZE_0; 	 //specify the peripheral size
	DMA1_Channel1->CCR &=~(DMA_CCR_PSIZE_1); //specify the peripheral size
	DMA1_Channel1->CCR|= DMA_CCR_MSIZE_0; 	 //specify the memory size
	DMA1_Channel1->CCR &=~(DMA_CCR_MSIZE_1); //specify the memory size

	DMA1_Channel1->CCR |= DMA_CCR_EN; // enabling the DMA
}

int main(void)
{



}
