// DS1722.c
// Wava Chan
// wchan@g.hmc.edu
// October 2025
// Decode data and provide functions for the DS1722 temp sensor

#ifndef DS1722_H
#define DS1722_H

#include <stdint.h>
#include <stm32l432xx.h>

float decodeData(int msb, char lsb); 

#endif