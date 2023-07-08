
#include <stdint.h>
#include"stm32f1xx.h"
#include "I2C.h"
/*
 We will be using the I2C 2 of the STM32 which uses pin PB10 and PB11

1. Enable the clock on GPIO, I2C peripheral and also the clock for alternate function.
2. PB10 AND PB11 are configured as alternate functions with open drain
To Set the MCU as master mode
Disable the peripheral from CR1 register before configuring other bits
3. Set the clock frequency in the I2C CR2 register with the frequency at which our main clock is running
4. Set the value in CCR(clock control register) to set the SCL frequency
5. Configuring the I2C Trise register to set the rise time (and fall time) of SCL
Enable the peripheral from CR1 register
6. creating functions for start condition of Transmit and Receive
*/




int main(void)
{
    while(1)
    {

    }
}

