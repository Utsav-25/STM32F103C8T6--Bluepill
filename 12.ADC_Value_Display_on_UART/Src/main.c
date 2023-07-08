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
void SysTick_init()//using SysTick timer for generating delays
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

void USART_PinConfig()
{
	//enable the clock for USART2 and GPIOA pins
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Enable the clock to USART2
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //Enable clock to GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //Enable the Alternate function clock

	//configure the pin PA2 as output(TX) for alternate function
	GPIOA->CRL |= ( GPIO_CRL_MODE2_1 | GPIO_CRL_MODE2_0 ); //setting output speed of PA2 to max 50Mhz
	GPIOA->CRL |= GPIO_CRL_CNF2_1;    //setting as Alternate function output push pull
	GPIOA->CRL &= ~(GPIO_CRL_CNF2_0); //setting as Alternate function output push pull
	//configure the pin PA3 as input(RX) for alternate function
	// As the defaults of PA3 are set as input, we do not need to make additional changes
}
void USART_BaudRate()
{
	USART2->BRR = 0xd05; //for baud of 9600 at 32Mhz bus clock.
}
void USART_Init()
{
	USART_PinConfig(); //initialize the pins PA2 and PA3 as Tx and Rx
	//Set up the baud rate
	USART_BaudRate();
	//Enable the USART, Transmit and Receive
	USART2->CR1 |= USART_CR1_UE; //enable the USART
	//setting the word length
	USART2->CR1 &= ~(USART_CR1_M); // 1 start bit, 8 data bits and n stop bit
	USART2->CR1 |= USART_CR1_TE;   //USART2 transmit enable
	USART2->CR1 |= USART_CR1_RE;   //USART2 transmit enable

}
void USART_TransmitByte(uint8_t byte)
{
	while(!(USART2->SR & USART_SR_TXE)); // we are checking if the transmit data register is empty
	USART2->DR = byte; //loading the contents to the data register
	while(!(USART2->SR & USART_SR_TC)); // checking if the data transmit has been completed by checking the status of TC bit
}
void USART_NewLine()
{
	USART_TransmitByte('\n');
}
int USART_TransmitNumber(uint32_t val)
{
	unsigned char buf[5];
	int8_t ptr;
	for(ptr=0;ptr<5;++ptr)
	{
		buf[ptr] = (val % 10) + '0';
		val /= 10;
	}
	for(ptr=4;ptr>0;--ptr)
	{
		if (buf[ptr] != '0')
		break;
	}
	for(;ptr>=0;--ptr)
	{
		USART_TransmitByte(buf[ptr]);
	}
}


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

int val=0; // to store the ADC value

int main(void)
{
	system_clk();
	SysTick_init();
	USART_Init();
	ADC_init();

	while(1)
	{
		val=ADC_Read();
		USART_Init();
		USART_TransmitNumber(val);
		USART_NewLine();
		delay_ms(100);
	}
}


