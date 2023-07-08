#include <stdint.h>
#include "stm32f1xx.h"
/*
We will be using the SPI1 of the STM2f103.
We will look at Simple initialization of SPI1 and transmission of a character over logic analyzer
The pin designation by default is as follows:
PA4-> Chip select (CS)
PA5-> SCLK
PA6-> MISO
PA7-> MOSI
The following are the steps to initialize and set the registers for SPI1:

1. Enable the alternate function for pins in the RCC register
2. Enable the clock to the SPI1 peripheral and GOIOA port
3. Initializing the pins as either input of output based on function
	PA4-output, general purpose push pull
4. Set SPI1 in master mode in CR1 register (as the microcontroller will be a master in this case)
5. Set the baud rate in CR1 register at which the clock signal will be generated.
	We will choose BR= Fpclk/256 , which is the slowest, just to be on the safe side
6. Enable the SSOE in CR2 register (SSOE is slave select output enable)
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

void SPI1_Init()
{
	//1. Enable the alternate function for the pins
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //enabling alternate function
	//2. Enable clock to SPI1 peripheral and GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //enable SPI1 clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //enable GPIOA clock
	//3. Initializing the modes of SPI pins
		//PA4 or Chip Select is used as a general purpose push pull pin
	GPIOC->CRL |= (GPIO_CRL_MODE4_1 | GPIO_CRL_MODE4_0); // enable max speed to 50Mhz.
	GPIOC->CRL &= ~(GPIO_CRL_CNF4_1 | GPIO_CRL_CNF4_0 ); //general purpose Push Pull.
		//PA5 or SCLK is used as an alternate function output push pull pin
	GPIOC->CRL |= (GPIO_CRL_MODE5_1 | GPIO_CRL_MODE5_0); // enable max speed to 50Mhz.
	GPIOC->CRL &= ~(GPIO_CRL_CNF5_0 ); //alternate function Push Pull.
	GPIOC->CRL |= (GPIO_CRL_CNF5_1 );  //alternate function Push Pull.
		//PA7 or MOSI is used as an alternate function output push pull pin
	GPIOC->CRL |= (GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0); // enable max speed to 50Mhz.
	GPIOC->CRL &= ~(GPIO_CRL_CNF7_0 ); //alternate function Push Pull.
	GPIOC->CRL |= (GPIO_CRL_CNF7_1 );  //alternate function Push Pull.
		//PA6 or MISO is used as an alternate function input
	GPIOC->CRL &= ~(GPIO_CRL_MODE6_1 | GPIO_CRL_MODE6_0); // default input mode.
	GPIOC->CRL &= ~(GPIO_CRL_CNF6_0 ); //input with pull up pull down.
	GPIOC->CRL |= (GPIO_CRL_CNF6_1 );  //input with pull up pull down.

	//4. Set SPI1 in master mode
	SPI1->CR1 |= SPI_CR1_MSTR; //set STM32 as master
	//5. Set the baud rate
	SPI1->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2; // setting the baud rate
	//6. Enable the SSOE bit
	SPI1->CR2 |= SPI_CR2_SSOE; // enable SSOE
	GPIOA->ODR |= (GPIO_ODR_ODR4); // pull chip select pin high
	//7. Enable the SPI peripheral
	SPI1->CR1 |= SPI_CR1_SPE; // enable the SPI peripheral


}


int main(void)
{
	system_clk();
	SPI1_Init();

	while(1)
	{
		// To transmit the data, we do the following steps:
			//1. We will first pull the CS pin low
			//2. Put the data into the SPI1 data register
			//3. Wait until the SPI1 is in busy state by checking the status register
			//4. Pull Chip select pin high again
		GPIOA->ODR &= ~(GPIO_ODR_ODR4); // pull chip select pin low
		SPI1->DR = 'A'; // transmitting a character
		while(SPI1->SR & SPI_SR_BSY){} // while the status is busy, wait
		GPIOA->ODR |= (GPIO_ODR_ODR4); // pull chip select pin high


	}


}
