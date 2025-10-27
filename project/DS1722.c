// DS1722.c
// Wava Chan
// wchan@g.hmc.edu
// October 2025
// Decode data and provide functions for the DS1722 temp sensor

#include "DS1722.h"

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