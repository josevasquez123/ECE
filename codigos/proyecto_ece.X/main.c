/*
 * File:   main.c
 * Author: josem
 *
 * Created on May 11, 2020, 1:40 PM
 */


#include <xc.h>
#include "config.h"
#include "abecedario.h"
#include "glcd.h"

void config(void);

void main(void) {
    
    
    config();
    glcd_on();
    glcd_clearscreen();
    
    write_small(0, 0, "TIPOS DE COMUNICACION",0);
    write_small(12, 0, "1) UART",0);
    write_small(24, 0, "2) I2C",0);
    write_small(36, 0, "3) SPI",0);
    write_small(48, 0, "4) MODBUS",0);

    
    
    while(1){
    }
    
    return;
}


void config(void){
    //Set all LCD pins as OUTPUT
    DATAPORT = 0x00;
    RSPORT = 0;
    RWPORT = 0;
    EPORT = 0;
    CS1PORT = 0;
    CS2PORT = 0;
    RSTPORT = 0;
    RST = 1;
}