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
	USART2->CR1 |= USART_CR1_RE;   //USART2 receive enable

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

int val=0;
int main(void)
{
    system_clk();
    timer2_init();
    USART_Init();

    while(1)
    {
    	USART_TransmitSrting("Hello World!");
    	USART_TransmitNumber(val);
    	USART_NewLine();
    	delay_ms(1000);
    	val++;
    }
}
