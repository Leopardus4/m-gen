/*
File:       lpc111x.c
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

#include "lpc111x.h"


/*---------------------------------------------------*/


// local declarations


// Converts port & pin into LPC_IOCON regisrer for gpio
static void lpc111x_getIoconReg(char reg[20], unsigned int* gpioFunc, const int port, const int pin, int inFp_line);

// Prints macro for 1 pin
static int lpc111x_printmacro(FILE* outFp, char mode, int port, int pin, const char* ioconReg, unsigned int gpioFunc, const char* name, const char* comment);



/*---------------------------------------------------*/

//returning pointers to functions
void lpc111x_getData(TARGET_ATTRIBUTES* atrs)
{
    char desc[DESCRIPTION_LENGTH] = "NXP LPC1110-1115 32-bit ARM Cortex M0 microcontrollers";

    strncpy(atrs->description, desc, DESCRIPTION_LENGTH);

    atrs->help      =  &lpc111x_help;
    atrs->init      =  &lpc111x_init;
    atrs->macroGen  =  &lpc111x_generateMacros;
}


/*---------------------------------------------------*/


//writing skelet of input file
void lpc111x_init(FILE* fp, const TARGET_FLAGS* fls)
{
    /*
    Creating macros section and 'other' section for LPC

    Format:

        $m
        Mode PORT PIN name Comment

        i   2   4   led1   Exemplary comment...

        $o
        Some info how to use this sheet (avaiable modes etc... )

    */

    fprintf(fp, "$m" "\n");
    fprintf(fp, "Mode PORT PIN Name Comment\n\n");
    fprintf(fp, "l   2   4   led_status    Write here your own macros...\n\n");


    fprintf(fp, "$o" "\n");

    fprintf(fp,
        "Format:                                                            \n"
        "   PORT:                                                           \n"
        "   LPC port number in format: (i. e.) '2'                          \n"
        "                                                                   \n"
        "   PIN:                                                            \n"
        "   LPC pin number (in PORT) in format: (i. e.) '4'                 \n"
        );
}


/*---------------------------------------------------*/


//converting array from input file to macros in output file
int  lpc111x_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls)
{
    int macrosNum = 0;


    /*
    Data for macros
    */

    char mode[3];   // mode of pin

    int port;   // LPC port
    int pin;    // LPC pin

    char lpc_iocon_reg[20]; // LPC_IOCON->register_name
    unsigned int gpioFunc;  // representation of GPIO function in IOCON_PIOx_x register

    char name[GM_PINNAME_LENGTH];         //symbolic name of pin (defined by user)
    char comment[GM_COMMENT_LENGTH];      //user comment about function of pin



    // reading beginning of section
    // - one 'Enter' , and
    // first line - heading - unwanted
    fgets(comment, sizeof(comment), inFp);
    fgets(comment, sizeof(comment), inFp);



    /*
    A couple of useful #defines
    */


    /* bit masks */
    fprintf(outFp, "#define gm_SYSAHBCLKCRTL_IOCON  (1<<16)\n\n");

    fprintf(outFp, "#define gm_DIGITALMODE          (1<<7)\n\n");

    fprintf(outFp, "#define gm_PULLUP               (1<<4)\n\n");


    /* Before any operations with gpio, clock for IOCON block must be enabled */
    fprintf(outFp, "\n\n/* gpio_enableAccess() must be used before any other macros for all gpios */\n\n");

    fprintf(outFp, "#define gpio_enableAccess() \\\n    do{LPC_SYSCON->SYSAHBCLKCTRL |= gm_SYSAHBCLKCRTL_IOCON;} while(0)\n\n");


    fprintf(outFp, "\n//------------------------------------------------------------------------//\n\n");





    // reading array line by line
    do{

        if( 4 != fscanf(inFp, " %2s %d %d %127s", mode, &port, &pin, name))
        {
            //end of section
            if(mode[0] == '$')
                break;

            else
            {
                message(ERR, "Input file cannot be correctly read\n");
                return -1;
            }

        }

        //end of section
        if(mode[0] == '$')
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




        //Formatting

        mode[0] = tolower(mode[0]);


        if( port > 3 || port < 0 )
        {
            message(ERR, "Bad PORT: %d\n", port);
            return -1;
        }


        if( pin > 11 || pin < 0 )
        {
            message(ERR, "Bad PIN: %c\n", pin);
            return -1;
        }


        // getting name of iocon register and GPIO function representation
        lpc111x_getIoconReg(lpc_iocon_reg, &gpioFunc, port, pin, getActualLine(inFp) );



        // PIO0_4 and 0_5 - open drain
        if(port==0 && (pin==4 || pin==5))
        {
            if(mode[0]=='o' || mode[0]=='d')
                message(WARN,   "PIO%d_%d is open drain output\n"
                                "\t(line: %d )\n", port, pin, getActualLine(inFp));

            else if(mode[0]=='h')  //it's impossible to drive active-high actuator
            {
                message(ERR,    "PIO%d_%d is ONLY open drain output (only active-low mode avaiable)\n"
                                "\t(line: %d )\n", port, pin, getActualLine(inFp));

                return -1;
            }
        }

        // PIO0_1 - BOOT pin
        if(port==0 && pin==1)
        {
            message(WARN, "PIO0_1 is 'bootloader select' pin\n");
        }




        //creating set of macros for 1 gpio

        if(lpc111x_printmacro(outFp, mode[0], port, pin, lpc_iocon_reg, gpioFunc, name, comment) != 0)
            return -1;



        ++macrosNum;

    } while(1);



    return macrosNum;
}


/*---------------------------------------------------*/


//some informations
void lpc111x_help(void)
{
    printf("NXP LPC111x family ( ARM Cortex M0 core )                               \n"
           "                                                                        \n"
           "GPIO are grouped in 4 ports (0-3).                                      \n"
           "Each port contain up to 12 pins (0-11).                                 \n"
           "See part datasheet for details.                                         \n"
           "                                                                        \n"
           "Note that there are some specific pins:                                 \n"
           "    PIO0_0  - RESET pin                                                 \n"
           "    PIO0_1  - BOOTLOADER pin                                            \n"
           "                                                                        \n"
           "    PIO0_10 - SWCLK                                                     \n"
           "    PIO1_3  - SWDIO                                                     \n"
           "                                                                        \n"
           "    PIO0_4 and PIO0_5 - open drain outputs                              \n"
           "                                                                        \n"
           "                                                                        \n"
           "                                                                        \n"
           "This module supports lpc111x MCU-s.                                     \n"
           "Other chips (11Cxx, 110x, 112x, ...) has not been tested                \n"
           "and may required some modifications. If you are interested,             \n"
           "it's possible to easy add new module for m-gen. See:                    \n"
           "    https://github.com/Leopardus4/m-gen/blob/master/CONTRIBUTORS.md     \n"
           "                                                                        \n"
           );
}



/*---------------------------------------------------*/

// Converts port & pin into LPC_IOCON regisrer for gpio and function bits in regisrer
void lpc111x_getIoconReg(char reg[20], unsigned int* gpioFunc, const int port, const int pin, int inFp_line)
{

    // special pins

    if( port == 0 && pin == 0)
    {
        snprintf(reg, 20, "RESET_PIO0_0");
        *gpioFunc = 0x1;

        message(WARN,   "Pin PIO%d_%d is the RESET pin\n"
                            "\t(line: %d )\n", port, pin, inFp_line);
    }


    else if( port == 0 && pin == 10)
    {
        snprintf(reg, 20, "SWCLK_PIO0_10");
        *gpioFunc = 0x1;

        message(WARN,   "Pin PIO%d_%d is the SWCLK debug pin\n"
                            "\t(line: %d )\n", port, pin, inFp_line);
    }

    else if( port == 1 && pin == 3)
    {
        snprintf(reg, 20, "SWDIO_PIO1_3");
        *gpioFunc = 0x1;

        message(WARN,   "Pin PIO%d_%d is the SWDIO debug pin\n"
                            "\t(line: %d )\n", port, pin, inFp_line);
    }



    // pins: R_PIOx_y

    else if( port == 0 && pin == 11)
    {
        snprintf(reg, 20, "R_PIO0_11");
        *gpioFunc = 0x1;
    }

    else if( port == 1 && pin == 0)
    {
        snprintf(reg, 20, "R_PIO1_0");
        *gpioFunc = 0x1;
    }

    else if( port == 1 && pin == 1)
    {
        snprintf(reg, 20, "R_PIO1_1");
        *gpioFunc = 0x1;
    }

    else if( port == 1 && pin == 2)
    {
        snprintf(reg, 20, "R_PIO1_2");
        *gpioFunc = 0x1;
    }


    // normal pins

    else
    {
        snprintf(reg, 20, "PIO%d_%d", port, pin);
        *gpioFunc = 0x0;
    }
}




/*---------------------------------------------------*/

// Prints macro for 1 pin
int lpc111x_printmacro(FILE* outFp, char mode, int port, int pin, const char* ioconReg, unsigned int gpioFunc, const char* name, const char* comment)
{
    switch(mode)
    {
        case 'i':   //Digital input

            fprintf(outFp, "/* %s - %s - digital input \n\t %s */\n\n", name, ioconReg, comment);

            fprintf(outFp, "#define %s_dirIn()      do{LPC_IOCON->%s = gm_DIGITALMODE | (%d<<0);} while(0)\n\n", name, ioconReg, gpioFunc);


            fprintf(outFp, "#define %s_isHigh()     ((LPC_GPIO%d->DATA & (1<<%d)) != 0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_isLow()      ((LPC_GPIO%d->DATA & (1<<%d)) == 0)\n\n", name, port, pin);

            break;


        case 'o':   //Digital output

            fprintf(outFp, "/* %s - %s - digital output \n\t %s */\n\n", name, ioconReg, comment);

            fprintf(outFp, "#define %s_dirOut()     do{LPC_IOCON->%s = gm_DIGITALMODE | (%d<<0); \\\n"
                           "    LPC_GPIO%d->DIR |= (1<<%d);} while(0)\n\n", name, ioconReg, gpioFunc, port, pin);


            fprintf(outFp, "#define %s_setHigh()    do{LPC_GPIO%d->DATA |= (1<<%d);} while(0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_setLow()     do{LPC_GPIO%d->DATA &= ~(1<<%d);} while(0)\n\n", name, port, pin);

            break;


        case 'd':   //Input and output

            fprintf(outFp, "/* %s - %s - digital input and output \n\t %s */\n\n", name, ioconReg, comment);


            fprintf(outFp, "#define %s_init()       do{LPC_IOCON->%s = gm_DIGITALMODE | (%d<<0);} while(0)\n\n", name, ioconReg, gpioFunc);

            fprintf(outFp, "#define %s_dirIn()      do{LPC_GPIO%d->DIR &= ~(1<<%d);} while(0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_dirOut()     do{LPC_GPIO%d->DIR |= (1<<%d);} while(0)\n\n\n", name, port, pin);


            fprintf(outFp, "#define %s_isHigh()     ((LPC_GPIO%d->DATA & (1<<%d)) != 0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_isLow()      ((LPC_GPIO%d->DATA & (1<<%d)) == 0)\n\n\n", name, port, pin);


            fprintf(outFp, "#define %s_setHigh()    do{LPC_GPIO%d->DATA |= (1<<%d);} while(0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_setLow()     do{LPC_GPIO%d->DATA &= ~(1<<%d);} while(0)\n\n", name, port, pin);

            break;



        case 'l':   //Active low output

            fprintf(outFp, "/* %s - %s - active low output \n\t %s */\n\n", name, ioconReg, comment);

            fprintf(outFp, "#define %s_asOutput()   do{LPC_IOCON->%s = gm_DIGITALMODE | (%d<<0); \\\n"
                           "    LPC_GPIO%d->DIR |= (1<<%d); LPC_GPIO%d->DATA |= (1<<%d);} while(0)\n\n", name, ioconReg, gpioFunc, port, pin, port, pin);


            fprintf(outFp, "#define %s_On()         do{LPC_GPIO%d->DATA &= ~(1<<%d);} while(0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_Off()        do{LPC_GPIO%d->DATA |= (1<<%d);} while(0)\n\n", name, port, pin);

            break;


        case 'h':   //Active high output

            fprintf(outFp, "/* %s - %s - active high output \n\t %s */\n\n", name, ioconReg, comment);

            fprintf(outFp, "#define %s_asOutput()   do{LPC_IOCON->%s = gm_DIGITALMODE | (%d<<0); \\\n"
                           "    LPC_GPIO%d->DIR |= (1<<%d); LPC_GPIO%d->DATA &= ~(1<<%d);} while(0)\n\n", name, ioconReg, gpioFunc, port, pin, port, pin);


            fprintf(outFp, "#define %s_Off()        do{LPC_GPIO%d->DATA &= ~(1<<%d);} while(0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_On()         do{LPC_GPIO%d->DATA |= (1<<%d);} while(0)\n\n", name, port, pin);

            break;


        case 'b':   //Button type - active low input with internall pull-up

            fprintf(outFp, "/* %s - %s - active low input with internal pull-up resistor \n\t %s */\n\n", name, ioconReg, comment);

            fprintf(outFp, "#define %s_asInput()    do{LPC_IOCON->%s = gm_DIGITALMODE | gm_PULLUP | (%d<<0);} while(0)\n\n", name, ioconReg, gpioFunc);


            fprintf(outFp, "#define %s_isActive()   ((LPC_GPIO%d->DATA & (1<<%d)) == 0)\n\n", name, port, pin);

            fprintf(outFp, "#define %s_isInactive() ((LPC_GPIO%d->DATA & (1<<%d)) != 0)\n\n", name, port, pin);

            break;



        default:
            message(ERR, "Unknown mode: %c\n", mode);
            return 1;
    }


    // for better look
    fprintf(outFp, "\n//------------------------------------------------------------------------//\n\n");

    return 0;

}
