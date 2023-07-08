#include<stm32f1xx.h>
#include <stdint.h>

/* We will be using USART2 of the STM32F103 which uses the PA2 and PA3 as Tx and Rx
Here, PA2->TX and PA3->RX. Both of these are NOT 5V tolerant so we need to be careful while using external USB to Serial converter

1. Setting up the pins for alternate function and input or output
	We need to enable the clock for USART2.
	Enable the clock for GPIOA.
	Set up the GPIO for alternate function.

2. Set up the USART baud rate. This is determined with respect to the clock signal of the bus
	the formula as per reference manual= Tx/Rx baud = (Fclk)/(16* USARTDIV), where USART dic is set in the
	BRR register (baud rate register)

3. Set up the Control register for the USART and create function to transmit data

*/

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
	// we will be setting up a baud rate of 9600. and have a bus clock of 32 Mhz
	//So value of USARTDIV = (32Mhz)/(16*9600) = 208.33
	//Value of BRR= USARTDIV/16 = Mantissa Part
	//				USARTDIV%16 = Fractional part rounded to nearest whole number
	//Mantissa part = 208 = 0xd0 in Hex
	//Fractional part= 0.33*16 = 5.28 = 5(rounded off to whole number)= 5 in hex
	//For 9600 Baud USARTDIV= 0xd05.
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

//CREATING FUNCTIONS TO TRANSMIT AND RECEIVE DATA

void USART_TransmitByte(uint8_t byte)
{
	while(!(USART2->SR & USART_SR_TXE)); // we are checking if the transmit data register is empty
	USART2->DR = byte; //loading the contents to the data register
	//while(!(USART2->SR & USART_SR_TC)); // checking if the data transmit has been completed by checking the status of TC bit
}

uint16_t USART_ReceiveByte(uint8_t byte)
{
	while(!(USART2->SR & USART_SR_RXNE)); // we are checking if the data is received. RXNE is 1 when read data register is not empty
	return USART2->DR;
}

void USART_TransmitSrting( char *myString) // we are using a string pointer
{
	while(*myString) //passing value by reference
		USART_TransmitByte(*myString++); // transmit the byte pointer by myString pointer
	while(!(USART2->SR & USART_SR_TC)); // checking if the data transmit has been completed by checking the status of TC bit
}

void USART_NewLine()
{
	USART_TransmitByte('\n');
}
int main(void)
{

}
