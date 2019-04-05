/*
File:       lpc111x.c
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

#include "lpc111x.h"




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
    fprintf(fp, "l\t" "2\t" "4\t" "led_status\t" "Write here your own macros...\n\n");


    fprintf(fp, "$o" "\n");

    fprintf(fp,
        "Format:                                                            \n"
        "   PORT:                                                           \n"
        "   LPC port number in format: (i. e.) 'PIO2' or '2'                \n"
        "                                                                   \n"
        "   PIN:                                                            \n"
        "   LPC pin number (in PORT) in format: (i. e.) '4'                 \n"
        );
}


/*---------------------------------------------------*/


//converting array from input file to macros in output file
int  lpc111x_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls)
{
    message(ERR, "LPC111x module is not complete yet!\n");
    return -1;
}


/*---------------------------------------------------*/


//some informations
void lpc111x_help(void)
{
    printf("Warning: untested!\n");
}


