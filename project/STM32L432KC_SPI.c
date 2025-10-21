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
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 0b0101); //PA5 AF5 = SPI1_SCK
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL6, 0b0101); //PA6 AF5 = SPI1_MISO
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL7, 0b0101); //PA7 AF5 = SPI1_MOSI


    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);//set baud rate 
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol); //set clock polarity
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha); //set clock phase
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111);//configure data size tp 8 bit
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_LSBFIRST, 0); //enable MSB first
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_SSM, 1);//enable peripheral 
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_MSTR, 1);//enable controller 

    SPI1->CR2 |= _VAL2FLD(SPI_CR2_FRXTH, 1); //set FIFO to 8bits
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_SSOE, 1); //SS output is enabled
    
    //enable SPI
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_SPE, 0b1);

}

char spiSendReceive(char send){
    while(!(SPI1->SR & SPI_SR_TXE)){}; // wait for empty transmit buffer
    
    volatile char* address = (volatile char*)&SPI1->DR; // send data when it is written to a data register 
    *address = send; 

    //while(!(SPI_SR_RXNE & SPI1->SR)){}; //wait for data to be recieved 

    return *address; //return the receieve data
}

float decodeData(int msb, char lsb){
    //get main data 
    float temp = msb & 0b01111111; //mask data
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