/*
File: main.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19

edited by Wava Chan
Email: wchan@hmc.edu
Date: 10/17/2025
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";
char tempRes = "<p>Temperature Resolution:</p><form action =\"8bit\"><input type=\"submit\" value=\"1C Resolution (8 bits)\"></form>\
  <form action =\"9bit\"><input type=\"submit\" value=\"0.5C Resolution (9 bits)\"></form>
  <form action =\"10bit\"><input type=\"submit\" value=\"0.25C Resolution (10 bits)\"></form>
  <form action =\"11bit\"><input type=\"submit\" value=\"0.125 Resolution (11 bits)\"></form>
  <form action =\"12bit\"><input type=\"submit\" value=\"0.0625C Resolution (12 bits)\"></form>";
char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}

int res = 8; //automatically 8bit resolution
int updateTempRes(char request[]){
  if(inString(request, "8bit") == 1){
    res = 8;
  }
  else if(inString(request, "9bit") == 1){
    res = 9;
  }
  else if(inString(request, "10bit") == 1){
    res = 10;
  }
  else if(inString(request, "11bit") == 1){
    res = 11;
  }
  else if(inString(request, "12bit") == 1){
    res = 12;
  }
  else {
    res = 8;
  }
}

//TODO: function to send characters to laptop

/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////

int main(void) {
  configureFlash();
  configureClock();

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(PB3, GPIO_OUTPUT);
  
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  // TODO: Add SPI initialization code
  //initialize SPI w/ 200kHz br
  initSPI(200000, 0, 0);

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }

    // TODO: Add SPI code here for reading temperature

    //give GPIO access to SPI
    //get data from core to SPI

    //data to write
    int msb;
    char lsb, rs; //rs = resolution
    res = updateTempRes(request);

    //set resolution
    switch(res){
      case 8:
        rs = 0b11100000;
        break;
      case 9:
        rs = 0b11100010; 
        break;
      case 10:
        rs = 0b11100100; 
        break;
      case 11:
        rs = 0b11100110; 
        break;
      case 12:
        rs = 0b11101110; 
        break;
      default: 
        rs = 0b1100000;
        break;
    }

    //configure sensor 
    digitalWrite(CS, PIO_HIGH); 
    spiSendReceieve(0x80); 
    spiSendRecieve(rs);

    //end message
    digitalWrite(CS, PIO_LOW);
    digitalWrite(CS, PIO_HIGH); 
    
    //read data
    spiSendReceive(0x20); //read the MSB register
    msb = spiSendReceive(0x00);

    digitalWrite(CS, PIO_LOW);
    digitalWrite(CS, PIO_HIGH);

    spiSendReceive(0x01); //read the LSB register
    lsb = spiSendRecieve(0x00);

    //decode data
    float tempData = decodeData(msb, lsb);

    // Update string with current LED state
  
    int led_status = updateLEDStatus(request);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    //Update with temp read
    char temp[20];
    //char cels[20];
    sprintf(temp, "%f", tempData);
    //sprintf(cels, "degrees Celcius");

    // finally, transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED

    sendString(USART, "<h2>LED Status</h2>");


    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    sendString(USART, tempRes); //for updating temp resolution
    sendString(USART, "<h2>Temperature</h2>");
    sendString(USART, "<p>");
    sendString(USART, temp);
    //sendString()
    sendString(USART, "</p>");


  
    sendString(USART, webpageEnd);
  }
}
