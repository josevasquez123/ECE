/*
 * File:   glcd.c
 * Author: josem
 *
 * Created on May 11, 2020, 1:49 PM
 */


#include <xc.h>
#include "glcd.h"

volatile uint8 mmap[1024]; //Unused, but space blocked for later use as a memory map

void init_mmap(void) {
    for (int i = 0; i < 1024; i++) {
        mmap[i] = 0;
    }
}


void glcd_exec() {
    E = HIGH;
    __delay_us(1);
    E = LOW;
    __delay_us(1);
}

void glcd_reset(void) {
    RST = LOW; //Reset display
    __delay_ms(100);
    RST = HIGH;
    __delay_ms(100);
}

void glcd_on(void) {
    init_mmap();
    CS1 = HIGH; //Don't know why but these are needed otherwise display doesn't turn on
    CS2 = HIGH;
    E = LOW; //Set enable to low to turn on display
    __delay_ms(1);

    glcd_reset();
    RS = LOW; //Display on
    RW = LOW;
    DATA = 0b00111111;

    glcd_exec();
}

void glcd_gotoy(uint8 y) {
    RS = LOW; //Set Y Address (0-63)
    RW = LOW;
    DATA = 0b01000000 | y;

    glcd_exec();
}

void glcd_gotox(uint8 x) {
    RS = LOW; //Set X Address (0-7)
    RW = LOW;
    DATA = 0b10111000 | x;

    glcd_exec();
}

void glcd_putbyte(uint8 x) {
    RS = HIGH;
    RW = LOW;
    DATA = x;

    glcd_exec();
}

void glcd_clearscreen(void) {
    CS1 = HIGH;
    CS2 = HIGH;
    glcd_gotoy(0);
    for (uint8 x = 0; x < 8; x++) {
        glcd_gotox(x);
        for (uint8 y = 0; y < 64; y++) {
            glcd_putbyte(0);
        }
    }
    glcd_gotoy(0);
    glcd_gotox(0);
}

uint8 glcd_putchar(uint8 ch, uint8 x, uint8 y, uint8 clrflag) { //x: 0-63, y:0-127

    uint8 sy = 0;
    int start = (ch - 32)*55;
    uint8 vshift = x % 8;
    uint8 thebyte = 0;
    uint8 xloc = x / 8;
    uint8 xval = 0;
    uint8 mask = (0xFF << (8 - vshift)); //mask for leftmost bits
    uint8 lbits = 0;

    for (int i = 0; i < bigfont[start]*3; i += 3) {
        if (y >= 64 && y < 128) {
            sy = y - 64;
            CS1 = LOW;
            CS2 = HIGH;
        } else if (y < 64) {
            sy = y;
            CS1 = HIGH;
            CS2 = LOW;
        } else if (y > 128) {
            break;
        }

        xval = xloc;
        thebyte = bigfont[start + i + 1] << vshift;


        if (xval < 8 && thebyte > 0) {
            glcd_gotox(xval);
            glcd_gotoy(sy);
            glcd_putbyte(clrflag ? 0 : thebyte);
        }
        xval++;

        /* This check below is needed because otherwise xval < 8 won't work in the
         * step below. We are using an unsigned char.
         * If 0 is decreased by 1, it becomes 255 which then makes 255/8 = 31.
         * So incrementing 31 will give 32 which will never satisfy x<8.
         * So we rotate it to 0 so that text can disappear at the top of the
         * screen*/
        if (xval == 32) xval = 0;

        lbits = bigfont[start + i + 1] & mask;
        lbits = lbits >> (8 - vshift);
        thebyte = (bigfont[start + i + 2] << vshift) | lbits;


        if (xval < 8 && thebyte > 0) {
            glcd_gotox(xval);
            glcd_gotoy(sy);
            glcd_putbyte(clrflag ? 0 : thebyte);
        }
        xval++;
        
        if (xval == 32) xval = 0;

        lbits = bigfont[start + i + 2] & mask;
        lbits = lbits >> (8 - vshift);
        thebyte = (bigfont[start + i + 3] << vshift) | lbits;


        if (xval < 8 && thebyte > 0) {
            glcd_gotox(xval);
            glcd_gotoy(sy);
            glcd_putbyte(clrflag ? 0 : thebyte);
        }
        xval++;
        if (xval == 32) xval = 0;

        lbits = bigfont[start + i + 3] & mask;
        lbits = lbits >> (8 - vshift);
        thebyte = lbits;


        if (xval < 8 && thebyte > 0) {
            glcd_gotox(xval);
            glcd_gotoy(sy);
            glcd_putbyte(clrflag ? 0 : thebyte);
        }

        y++;
    }
    return y;
}

// @param x - vertical coordinate, range:0-63
// @param y - horizontal coordinate, range: 0-127

uint8 write(uint8 x, uint8 y, uint8 *txt, uint8 clrflag) {
    int i = 0;
    while (*(txt + i) != '\0') {
        y = glcd_putchar(*(txt + i), x, y, clrflag);
        i++;
    }
    return y;
}

uint8 glcd_putchar_small(uint8 ch, uint8 x, uint8 y, uint8 clrflag) {
    //x:0-7
    //y:0-127
    int start = (ch - 32)*7;
    //x=1;
    uint8 gx = x;
    uint8 gy = y;
    uint8 gyloc = 0;
    uint8 vshift = x % 8;
    uint8 gxactual = gx / 8;
    uint8 mask = 0xFF << (8 - vshift);
    uint8 thebyte = 0;

    for (int i = 0; i < smallfont[start]; i += 1) {
        if (y < 64) {
            gyloc = y;
            CS1 = HIGH;
            CS2 = LOW;
        } else if (y >= 64 && y < 128) {
            gyloc = y-64;
            CS1 = LOW;
            CS2 = HIGH;
        } else {
            break;
        }

        uint8 gxloc = gxactual;

        thebyte = smallfont[start + i + 1] << vshift;
        if (gxloc < 8 && thebyte > 0 ) {
            glcd_gotoy(gyloc);
            glcd_gotox(gxloc);
            glcd_putbyte(clrflag ? 0 : thebyte);
        }

        gxloc++;

        uint8 rbits = smallfont[start + i + 1] & mask;
        rbits = rbits >> (8 - vshift);
        thebyte = rbits;

        if (gxloc < 8 && thebyte > 0 ) {
            glcd_gotox(gxloc);
            glcd_gotoy(gyloc);
            glcd_putbyte(clrflag ? 0 : thebyte);
        }

        y++;
    }

    return y;

}


uint8 write_small(uint8 x, uint8 y, uint8 *txt, uint8 clrflag) {
    int i = 0;
    while (*(txt + i) != '\0') {
        y = glcd_putchar_small(*(txt + i), x, y, clrflag);
        i++;
    }
    return y;
}