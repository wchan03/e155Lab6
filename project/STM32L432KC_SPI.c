// STM32L432KC_SPI.c
// Wava Chan
// wchan@g.hmc.edu
// October 2025
// SPI Library for the STM32L432KC Microcontroller

#include "STM32L432KC_SPI.h"

void initSPI(int br, int cpol, int cpha){
    //turn on SPI clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; 

    // configure GPIO pins
    gpioEnable(SCK);
    gpioEnable(CIPO);
    gpioEnable(COPI);
    gpioEnable(CS);

    pinMode(SCK, GPIO_ALT);
    pinMode(CIPO, GPIO_ALT);
    pinMode(COPI, GPIO_ALT);
    pinMode(CS, GPIO_OUTPUT);

    //set alternate function 5
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 0b0101); //SCK
    //GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL1, 0b0101); //switch SCK pin bc it wasn't working before
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL6, 0b0101); //CIPO
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL7, 0b0101); //COPI

    //reset SPI
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_SPE, 0b0);

    //SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);//set baud rate 
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, 0b110);//set baud rate 
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol); //set clock polarity
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha); //set clock phase
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_LSBFIRST, 0b0); //enable MSB first
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_SSM, 0b0);//enable peripheral 
    //SPI1->CR1 |= _VAL2FLD(SPI_CR1_SSI, 0b1);//enable peripheral control
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_MSTR, 0b1);//enable controller 

    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111);//configure data size tp 8 bit
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_SSOE, 1); //SS output is enabled
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_FRXTH, 1); //set FIFO to 8bits
    
    //enable SPI
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_SPE, 0b1);

}

char spiSendReceive(char send){
    while(!(SPI1->SR & SPI_SR_TXE)){}; // wait for empty transmit buffer
    
    // send data when it is written to a data register 
    *(volatile char*)(&SPI1->DR) = send;

    while(!(SPI_SR_RXNE & SPI1->SR)){}; //wait for data to be recieved 

    return (SPI1->DR); //return the receieve data
}

float decodeData(int msb, char lsb){
    //get main data 
    float temp = msb & 0b01111111; //turn it into a float & remove sign bit
    //get the sign bit 
    int sign = msb & (1 << 7); //8th bit
    if(!sign) { //when positive, add values
        if(1 << 7 & lsb) temp = temp + 0.5;
        if(1 << 6 & lsb) temp = temp + 0.25;
        if(1 << 5 & lsb) temp = temp + 0.125;
        if(1 << 4 & lsb) temp = temp + 0.0625;
    }
    else { //negative
        temp = -(~(msb & 0b0111111) + 1);
        if(1 << 7 & lsb) temp = temp - 0.5;
        if(1 << 6 & lsb) temp = temp - 0.25;
        if(1 << 5 & lsb) temp = temp - 0.125;
        if(1 << 4 & lsb) temp = temp - 0.0625;

    }

    return temp;
}