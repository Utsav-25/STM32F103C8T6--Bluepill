#include "stm32f1xx.h"
#include "I2C.h"

uint32_t read_register;
void I2C2_pin_conf()
{
	//1. Enable the clock on GPIO, I2C peripheral and also the clock for alternate function.
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; //enable the clock to I2C 2
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //enable clock to GPIOB
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //Enable the Alternate function clock

	//2. PB10 AND PB11 are configured as alternate functions with open drain
		//configuring PB10
	GPIOB->CRH |= GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1; //Output Mode at max speed
	GPIOB->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_CNF10_0;   //Alternate Function Output open drain
		//configuring PB11
	GPIOB->CRH |= GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1; //Output Mode at max speed
	GPIOB->CRH |= GPIO_CRH_CNF11_1 | GPIO_CRH_CNF11_0;   //Alternate Function Output open drain

}

void I2C2_master_init()
{
	//Initializing the GPIO pins
	I2C2_pin_conf();
	//Ensuring the I2C peripheral is disabled before any configurations
	I2C2->CR1 &= ~(I2C_CR1_PE); // disabling the peripheral
	//3. Program the input peripheral clock in I2C_CR2 register for correct timings
		// our clock is running at 32Mhz so we need to write this in the register.
		//The bits must be configured with the APB clock frequency and our bus frequency is also 32MHz
		// 32 in hex = 0x20.
	I2C2->CR2 = 0x20;
	//4. Set the value in CCR(clock control register) to set the SCL frequency
		//We would like to set a value of 50Khz as SCL frequency
		// Ton =10us and Toff = 10us. Fclk= 32Mhz = 31.25 ns
		// CCR value = (10us)/(31.25ns) = 320 = 0x140 in hex
	I2C2->CCR = 0x140; // setting 50Khz as SCL frequency.
	//5. Configuring the I2C Trise register to set the rise time (and fall time) of SCL
		//the max allowed rise time is 1000ns
		// TRISE value = [(1000ns) / (1/Fclk)]+1
		// Here in our case TRISE = 1000/(1/32) +1  = 33. = 0x21 in hex
	I2C2->TRISE = 0x21; // setting the Trise time for SCL
	//Enabling the I2C peripheral
		I2C2->CR1 |= I2C_CR1_PE; // enabling the peripheral

}
	//6. creating functions for start condition of Transmit and Receive
void I2C_start_condition_w() // function for start condition of transmit
{
	// when we initiate the start condition, the micro-controller automatically gets into master mode
	I2C2->CR1 |= I2C_CR1_START;
	while(!(I2C2->SR1 & I2C_SR1_SB));  // wait until the start bit generation flag (SB) is set.
	// after transmission of start condition, the BTF (Byte transfer finished) bit is set
	// this needs to be cleared by reading the SR1 register followed by a write in data register DR
	//here we read the status register
	read_register = I2C2->SR1;
}

void I2C_start_condition_r() // function for start condition of receive
{
	//for receiving data in master mode we need to enable the acknowledge bit.
	// setting this bit as high will make the MCU send an acknowledge signal to the receiver that a data packet is received
	I2C2->CR1 |= I2C_CR1_ACK; // enable the acknowledge.
	I2C2->CR1 |= I2C_CR1_START;
	while(!(I2C2->SR1 & I2C_SR1_SB));  // wait until the start bit generation flag (SB) is set.
	// after transmission of start condition, the BTF (Byte transfer finished) bit is set
	// this needs to be cleared by reading the SR1 register followed by a write in data register DR
	//here we read the status register
	read_register = I2C2->SR1;
}

void I2C_address_send_w(uint8_t slave_address)
{
	//In 7-bit addressing mode,
	 //To enter Transmitter mode, a master sends the slave address with LSB reset.
	 //To enter Receiver mode, a master sends the slave address with LSB set

	//First we will write the data register with the slave address (this also clears the BTF flag)
	I2C2->DR =slave_address;
	while(!(I2C2->SR1 & I2C_SR1_TXE)); 	// wait until all data has been transferred from shift register to SDA line
	while(!(I2C2->SR1 & I2C_SR1_ADDR)); // wait until the address byte is successfully sent
	//after address is successfully sent, we need to clear the ADDR flag by software reading SR1 followed by SR2 register
	read_register = I2C2->SR1; //for clearing the ADDR bit
	read_register = I2C2->SR2; //for clearing the ADDR bit
}

void I2C_address_send_r(uint8_t slave_address)
{

	//I2C2->CR1 |= I2C_CR1_ACK;
	I2C2->DR = slave_address;//Write slave address on I2C data bus
	//while(!(I2C2->SR1 & I2C_SR1_TXE));
	while(!(I2C2->SR1 & I2C_SR1_ADDR))
	{
	if((I2C2->SR1 & I2C_SR1_AF) == 1) // if the acknowledge failure occurs
		{
		printf("error in sending read slave address\n");
		return 0;
		}
	}

	//after address is successfully sent, we need to clear the ADDR flag by software reading SR1 followed by SR2 register
	//Clearing the ADDR bit
	read_register = I2C2->SR1;
	read_register = I2C2->SR2;
}

void I2C_databyte_send(uint8_t data)
{
	I2C2->DR = data;
	while(!(I2C2->SR1 & I2C_SR1_TXE)); 	// wait until all data has been transferred from shift register to SDA line
}

//Functions to receive data

//to receive 1 byte
/*The steps to receive 1 byte of data are:
1.Clear the ACK bit
2.Clear the ADDR
3.Program the STOP bit (or stop condition)
4.Wait until the RXNE flag is set and read the data
*/
uint8_t I2C_master_receive_byte()
{
	uint8_t data;
	I2C2->CR1 &= ~I2C_CR1_ACK; //clear ACK bit
	I2C2->CR1 |= I2C_CR1_STOP; //generate stop condition as we will not be transmitting but receiving data
	while(!(I2C2->SR1 & I2C_SR1_RXNE)); //wait until the RXNE flag is set, this means there is data in the data register
	data = I2C2->DR; // store the value of Data register in a variable
	return data;
}
//to receive 2 byte
/*The steps to receive 2 bytes of data are:
1.Set the POS and ACK bits
2.Wait for the ADDR bit to be set
3. Clear the ADDR and ACK bits
4.Wait for the BTF (Byte transfer finished) bit to set
5.Program the STOP bit (or stop condition)
6.Read the Data Register(DR) twice
*/
void I2C_master_receive_2byte(uint8_t data1, uint8_t data2)
{
	I2C2->CR1 |= I2C_CR1_POS; // set the POS bit
	I2C2->CR1 &= ~I2C_CR1_ACK; // clear the acknowledge bit
	while(!(I2C2->SR1 & I2C_SR1_BTF)); // Wait for the BTF flag to set
	I2C2->CR1 |= I2C_CR1_STOP; // program the stop bit
	data1 = I2C2->DR; // read the data register once
	data2 = I2C2->DR; // read the data register twice
}

//for receiving more than 3 bytes
void datareive_string(char data[])
{
	I2C2->CR1 |= I2C_CR1_ACK;
	while(!(I2C2->SR1 & I2C_SR1_BTF));
	I2C2->CR1 &= I2C_CR1_ACK;
}

void I2C_master_stop_generation()
{
	I2C2->CR1 |= I2C_CR1_STOP;
}
