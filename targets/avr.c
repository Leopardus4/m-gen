/*
File:       avr.c
Project:    m-gen
Version:    1.0

Copyright (C) 2019 leopardus

This file is part of m-gen 
    https://github.com/Leopardus4/m-gen

m-gen is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3, 
as published by the Free Software Foundation.

m-gen is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
with m-gen. If not, see 
    http://www.gnu.org/licenses/


*/







#include <stdio.h>

#include <ctype.h>  // isalpha()
#include <string.h> // strncpy()

#include "m-gen.h"
#include "gm-common.h"

#include "avr.h"


//declaration
int avr_printMacro(FILE* outFp, char mode, char port, char pin, char* name, char* comment);



/*---------------------------------------------------*/


void avr_getData(TARGET_ATTRIBUTES* atrs)
{
    char desc[DESCRIPTION_LENGTH] = "Atmel ATtiny and ATmega 8 bit RISC microcontrollers";

    strncpy(atrs->description, desc, DESCRIPTION_LENGTH);

    atrs->help      =  &avr_help;
    atrs->init      =  &avr_init;
    atrs->macroGen  =  &avr_generateMacros;
}



/*---------------------------------------------------*/


void avr_init(FILE* fp, const TARGET_FLAGS* fls)
{
    FUNCINFO();

    /*
    Creating macros section and 'other' section for AVR.

    Format:

        $m
        Mode PORT PIN Comment

        i   B   4   Exemplary comment...

        $o
        Some info how to use this sheet (avaiable modes etc... )

    */


    fprintf(fp, "$m" "\n");
    fprintf(fp, "Mode PORT PIN Name Comment\n\n");
    fprintf(fp, "l\t" "B\t" "4\t" "led_status\t" "Write here your own macros...\n\n");


    fprintf(fp, "$o" "\n");

    fprintf(fp,
        "Format:                                                            \n"
        "   Mode: avaiable modes for AVR:                                   \n"
        "     digital gpio - i. e. communication with other digital chips   \n"
        "   i   digital Input                                               \n"
        "   o   digital Output                                              \n"
        "   d   Double (in+out)                                             \n"
        "\n"
        "     other modes:                                                  \n"
        "   b   Button              - for input buttons / sensors           \n"
        "          (it uses internal pull-up resistor)                      \n"
        "   l   active Low output   -  / for transistors / leds etc.        \n"
        "   h   active High output  - /                                     \n"
        "                                                                   \n"
        "   PORT:                                                           \n"
        "   AVR port name in format: (i. e.) 'PORTB', 'B', 'portb', or 'b'  \n"
        "                                                                   \n"
        "   PIN:                                                            \n"
        "   AVR pin number (in PORT) in format: (i. e.) 'PB4', '4' or 'pb4' \n"
        "                                                                   \n"
        "   Name:                                                           \n"
        "   Symbolic name for pin (i.e. function in project) without spaces \n"
        "                                                                   \n"
        "   Comment:                                                        \n"
        "   Few words about pin function in project                         \n"
        "   ( only one line - do not use 'Enter' )                          \n"
        "                                                                   \n"
        );

}





/*---------------------------------------------------*/



int avr_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls)
{
    FUNCINFO();

    int macrosNum = 0;


    //tablice przechowujące podane przez użytkownika nazwy
    char _mode[3];
    char _port[6];       //symbol portu, dla AVR będzie to np. "PORTB" lub krócej: "B"
    char _pin[4];        //numer pinu w danym porcie, dla AVR np. dla pinu PB2 będzie to "PB2" lub krócej "2"

    char mode, port, pin;

    char name[128];   //symboliczna nazwa pinu, np. LED4 - nie dłuższa niż 50 znaków
    char comment[512];    //objaśnienie dot. pinu, w stylu: służy do sygnalizacji tego i tego - nie dłuższy niż 500 znaków


    // one 'Enter' , and
    // first line - heading - unwanted
    fgets(comment, sizeof(comment), inFp);
    fgets(comment, sizeof(comment), inFp);




    // one line - one pin
    do{

        if( 4 != fscanf(inFp, " %2s %5s %3s %127s", _mode, _port, _pin, name))
        {
            fprintf(stderr, "Access error\n");
            return -1;
        }

        //end of section
        if(_mode[0] == '$')
            break;



        // Comment

        comment[sizeof(comment)-1] = 1;

        if(fgets(comment, sizeof(comment), inFp) == NULL)
        {
            fprintf(stderr, "Access error\n");
            return -1;
        }

        if(comment[sizeof(comment)-1] == 0)
        {
            fprintf(stderr, "Error: Too long comment\n");
            return -1;
        }



        /*
            Formatting ...
        */
        strToLower(_mode);

        mode = _mode[0];



        strToUpper(_port);

        if(_port[0] == 'P')     // i. e. "PORTB"
        {
            _port[0] = _port[4];
            _port[1] = '\0';
        }

        port = _port[0];

        if( ! isalpha(port) )
        {
            fprintf(stderr, "Error: bad PORT: %c\n", port);
            return -1;
        }



        strToUpper(_pin);

        if(_pin[0] == 'P')     // i. e. "PB3"
        {
            _pin[0] = _pin[2];
            _pin[1] = '\0';
        }

        pin = _pin[0];

        if( ! isdigit(pin) )
        {
            fprintf(stderr, "Error: bad PIN: %c\n", pin);
            return -1;
        }



        if( avr_printMacro(outFp, mode, port, pin, name, comment) )
            return -1;

        ++macrosNum;


    } while(_mode[0] != '$');


    return macrosNum;

}




/*
Very ugly function...
*/
int avr_printMacro(FILE* outFp, char mode, char port, char pin, char* name, char* comment)
{

    switch (mode)
    {
        case 'i':   //Digital input
            fprintf(outFp, "/* %s - P%c%c - digital input \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_dirIn()      do{DDR%c &= ~(1<<P%c%c); PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin, port, port, pin);


            fprintf(outFp, "#define %s_isHigh()     ( (PIN%c & (1<< P%c%c)) != 0 )\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_isLow()      ( (PIN%c & (1<< P%c%c)) == 0 )\n\n", name, port, port, pin);

            break;


        case 'o':   //Digital output
            fprintf(outFp, "/* %s - P%c%c - digital output \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_dirOut()     do{DDR%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);


            fprintf(outFp, "#define %s_setHigh()    do{PORT%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_setLow()     do{PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            break;


        case 'd':   //Input and output
            fprintf(outFp, "/* %s - P%c%c - digital input and output \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_dirIn()      do{DDR%c &= ~(1<<P%c%c); PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin, port, port, pin);

            fprintf(outFp, "#define %s_dirOut()     do{DDR%c |= (1<<P%c%c);} while(0)\n\n\n", name, port, port, pin);


            fprintf(outFp, "#define %s_isHigh()     ( (PIN%c & (1<< P%c%c)) != 0 )\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_isLow()      ( (PIN%c & (1<< P%c%c)) == 0 )\n\n\n", name, port, port, pin);


            fprintf(outFp, "#define %s_setHigh()    do{PORT%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_setLow()     do{PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            break;


        case 'l':   //Active low output
            fprintf(outFp, "/* %s - P%c%c - active low output \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_asOutput()   do{DDR%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);


            fprintf(outFp, "#define %s_On()         do{PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_Off()        do{PORT%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            break;



        case 'h':   //Active high output
            fprintf(outFp, "/* %s - P%c%c - active high output \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_asOutput()   do{DDR%c |= (1<<P%c%c); PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin, port, port, pin);


            fprintf(outFp, "#define %s_On()         do{PORT%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_Off()        do{PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            break;





        case 'b':   //Button type - active low input with internall pull-up
            fprintf(outFp, "/* %s - P%c%c - active low input with internal pull-up \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_asInput()    do{DDR%c &= ~(1<<P%c%c); PORT%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin, port, port, pin);


            fprintf(outFp, "#define %s_isActive()   ( (PIN%c & (1<< P%c%c)) == 0 )\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_isInactive() ( (PIN%c & (1<< P%c%c)) != 0 )\n\n", name, port, port, pin);

            break;


        default:
            fprintf(stderr, "Error: unknown mode: %c\n", mode);
            return 1;
    }

    // tylko pozycjonowanie
    fprintf(outFp, "\n//------------------------------------------------------------------------//\n\n");

    return 0;
}




/*---------------------------------------------------*/

void avr_help(void)
{

    printf("AVR help \n"
    "\n"
    "Atmel AVR MCUs have some gpio pins.\n"
    "They are grouped into ports (PORTA, PORTB, ...)\n"
    "Each port contains up to 8 pins (PA0-PA7, PB0-PB7, ...)\n"
    "Note that not every MCU has all pins in port and all ports\n"
    "  i. e. ATmega256 has up to 38 gpios, but ATtiny4 in SOT-23 package has only 4 gpios \n");
}





