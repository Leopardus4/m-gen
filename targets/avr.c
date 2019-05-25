/*
File:       avr.c
Project:    m-gen
Version:    1.1

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


//local function - prints macros for one pin
static int avr_printMacro(FILE* outFp, char mode, char port, char pin, char* name, char* comment, const TARGET_FLAGS* fls);



/*---------------------------------------------------*/


void avr_getData(TARGET_ATTRIBUTES* atrs)
{
    char desc[DESCRIPTION_LENGTH] = "Atmel ATtiny and ATmega 8 bit RISC microcontrollers";

    strncpy(atrs->description, desc, DESCRIPTION_LENGTH);

    atrs->help      =  &avr_help;
    atrs->init      =  &avr_init;
    atrs->macroGen  =  &avr_generateMacros;

    atrs->presentModes.compatibilityMode = true;
}



/*---------------------------------------------------*/


void avr_init(FILE* fp, const TARGET_FLAGS* fls)
{

    /*
    Creating macros section and 'other' section for AVR.

    Format:

        $m
        Mode PORT PIN name Comment

        i   B   4   led1   Exemplary comment...

        $o
        Some info how to use this sheet (avaiable modes etc... )

    */


    fprintf(fp, "$m" "\n");
    fprintf(fp, "Mode PORT PIN Name Comment\n\n");
    fprintf(fp, "l\t" "B\t" "4\t" "led_status\t" "Write here your own macros...\n\n");


    fprintf(fp, "$o" "\n");

    fprintf(fp,
        "Format:                                                            \n"
        "   PORT:                                                           \n"
        "   AVR port name in format: (i. e.) 'PORTB', 'B', 'portb', or 'b'  \n"
        "                                                                   \n"
        "   PIN:                                                            \n"
        "   AVR pin number (in PORT) in format: (i. e.) 'PB4', '4' or 'pb4' \n"
        );

}





/*---------------------------------------------------*/



int avr_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls)
{

    int macrosNum = 0;


    //data from input file
    char _mode[3];      // input / output / ...
    char _port[6];      // PORT name (i. e. PORTB or short form: 'B')
    char _pin[4];       // PIN nr - i. e. PB3 or '3'

    char mode, port, pin;   //as previous, but after conversion to one letter

    char name[GM_PINNAME_LENGTH];         //symbolic name of pin (defined by user)
    char comment[GM_COMMENT_LENGTH];      //user comment about function of pin



    // one 'Enter' , and
    // first line - heading - unwanted
    fgets(comment, sizeof(comment), inFp);
    fgets(comment, sizeof(comment), inFp);


    // compatibility mode - empty macro

    if(fls->compatibilityMode == true)
    {
        fprintf(outFp,  "\n\n"
                        "/* gpio_enableAccess() - empty macro\n"
                        "   Used only for compatibility with other MCUs\n"
                        "   ( configured by 'm-gen -c' flag )\n"
                        " */\n\n");

        fprintf(outFp, "#define gpio_enableAccess()    do{} while(0)\n\n");

        fprintf(outFp, "\n//------------------------------------------------------------------------//\n\n");

    }





    // one line - one pin
    do{

        if( 4 != fscanf(inFp, " %2s %5s %3s %127s", _mode, _port, _pin, name))
        {
            // end of section
            if(_mode[0] == '$')
                break;

            else
            {
                message(ERR, "Input file cannot be correctly read\n");
                return -1;
            }
        }


        // end of section
        if(_mode[0] == '$')
            break;


        // comment

        comment[sizeof(comment)-1] = 1;

        if(fgets(comment, sizeof(comment), inFp) == NULL)
        {
            message(ERR, "Input file cannot be correctly read\n");
            return -1;
        }

        if(comment[sizeof(comment)-1] == 0)
        {
            message(ERR, "Too long comment\n");
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
            message(ERR, "Bad PORT: %c\n", port);
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
            message(ERR, "Bad PIN: %c\n", pin);
            return -1;
        }



        if( avr_printMacro(outFp, mode, port, pin, name, comment, fls) )
            return -1;

        ++macrosNum;


    } while(_mode[0] != '$');


    return macrosNum;

}


/*---------------------------------------------------*/

/*
Ugly function...
*/
int avr_printMacro(FILE* outFp, char mode, char port, char pin, char* name, char* comment, const TARGET_FLAGS* fls)
{

    switch (mode)
    {
        case 'i':   //Digital input
            fprintf(outFp, "/* %s - P%c%c - digital input \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_dirIn()      do{DDR%c &= ~(1<<P%c%c); PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin, port, port, pin);


            fprintf(outFp, "#define %s_isHigh()     ( (PIN%c & (1<<P%c%c)) != 0 )\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_isLow()      ( (PIN%c & (1<<P%c%c)) == 0 )\n\n", name, port, port, pin);

            break;


        case 'o':   //Digital output
            fprintf(outFp, "/* %s - P%c%c - digital output \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_dirOut()     do{DDR%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);


            fprintf(outFp, "#define %s_setHigh()    do{PORT%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_setLow()     do{PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin);

            break;


        case 'd':   //Input and output
            fprintf(outFp, "/* %s - P%c%c - digital input and output \n\t %s */\n\n", name, port, pin, comment);


            if(fls->compatibilityMode == true)  // only if '-c' command line parameter was specified
                fprintf(outFp, "#define %s_init()       do{} while(0)\n\n", name);


            fprintf(outFp, "#define %s_dirIn()      do{DDR%c &= ~(1<<P%c%c); PORT%c &= ~(1<<P%c%c);} while(0)\n\n", name, port, port, pin, port, port, pin);

            fprintf(outFp, "#define %s_dirOut()     do{DDR%c |= (1<<P%c%c);} while(0)\n\n\n", name, port, port, pin);


            fprintf(outFp, "#define %s_isHigh()     ( (PIN%c & (1<<P%c%c)) != 0 )\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_isLow()      ( (PIN%c & (1<<P%c%c)) == 0 )\n\n\n", name, port, port, pin);


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
            fprintf(outFp, "/* %s - P%c%c - active low input with internal pull-up resistor \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "#define %s_asInput()    do{DDR%c &= ~(1<<P%c%c); PORT%c |= (1<<P%c%c);} while(0)\n\n", name, port, port, pin, port, port, pin);


            fprintf(outFp, "#define %s_isActive()   ( (PIN%c & (1<<P%c%c)) == 0 )\n\n", name, port, port, pin);

            fprintf(outFp, "#define %s_isInactive() ( (PIN%c & (1<<P%c%c)) != 0 )\n\n", name, port, port, pin);

            break;


        default:
            message(ERR, "Unknown mode: %c\n", mode);
            return 1;
    }

    // for better look
    fprintf(outFp, "\n//------------------------------------------------------------------------//\n\n");

    return 0;
}




/*---------------------------------------------------*/

void avr_help(void)
{

    message(MSG, "AVR help \n"
    "\n"
    "Atmel AVR MCUs have some gpio pins.\n"
    "They are grouped into ports (PORTA, PORTB, ...)\n"
    "Each port contains up to 8 pins (PA0-PA7, PB0-PB7, ...)\n"
    "Note that not every MCU has all pins in port and all ports\n"
    "  i. e. ATmega256 has up to 38 gpios, but ATtiny4 in SOT-23 package has only 4 gpios \n");
}





