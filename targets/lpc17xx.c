/*
File:		lpc17xx.c
Project:	m-gen
Version:	1.2

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



Changelog:

    1.2     Create lpc17xx.c
                as part of lpc17xx m-gen module

*/



#include <stdio.h>

#include <ctype.h>  // isalpha()
#include <string.h> // strncpy()

#include "m-gen.h"
#include "gm-common.h"

#include "lpc17xx.h"


/*---------------------------------------------------*/

/* structure to store string iterals for macro / inline function
*/
typedef struct{

    /* part for non-return functions */
    const char *mBegin;
    const char *mMid;
    const char *mEnd;

    /* part for conditionals (if-else) */
    const char *cmBegin;
    const char *cmMid;
    const char *cmEnd;

} MACRO_STRS;



static MACRO_STRS macros = {

    .mBegin = "#define ",
    .mMid   = "() \\\n    do{",
    .mEnd   = " } while(0)\n\n",

    .cmBegin = "#define ",
    .cmMid   = "() \\\n    (",
    .cmEnd   = ")\n\n"
};



static MACRO_STRS inlineF = {

    .mBegin = "static inline void ",
    .mMid   = "(void) {\n    ",
    .mEnd   = "\n}\n\n",

    .cmBegin = "static inline uint32_t ",
    .cmMid   = "(void) {\n    return (",
    .cmEnd   = ");\n}\n\n",
};



static MACRO_STRS *macroFmt;


/*---------------------------------------------------*/

// local function

// write set of macros for one pin
static int lpc17xx_printMacro(FILE* outFp, const char mode, unsigned int port, unsigned int pin, const char* name, const char* comment);


/*---------------------------------------------------*/


//return pointers to functions
void lpc17xx_getData(TARGET_ATTRIBUTES* atrs)
{
    char desc[DESCRIPTION_LENGTH] = "NXP LPC17xx series 32-bit ARM Cortex M3 microcontrollers";

    strncpy(atrs->description, desc, DESCRIPTION_LENGTH);

    atrs->help      =  &lpc17xx_help;
    atrs->init      =  &lpc17xx_init;
    atrs->macroGen  =  &lpc17xx_generateMacros;

    atrs->presentModes.compatibilityMode    = true;
    atrs->presentModes.inlineFunc           = true;
}



/*---------------------------------------------------*/

//write skelet of input file
void lpc17xx_init(FILE* fp, const TARGET_FLAGS* fls)
{
    /*
    Creating macros section and 'other' section for LPC
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


//convert array from input file to macros in output file
int  lpc17xx_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls)
{
    int macrosNum = 0;


    char mode[3];   // mode of pin

    int port;   // LPC port
    int pin;    // LPC pin


    char name[GM_PINNAME_LENGTH];         //symbolic name of pin (defined by user)
    char comment[GM_COMMENT_LENGTH];      //user comment about function of pin



    // 'inline' mode
    if(fls->inlineFunc == true)
        macroFmt = &inlineF;

    else
        macroFmt = &macros;



    // reading beginning of section
    // - one 'Enter' , and
    // first line - heading - unwanted
    fgets(comment, sizeof(comment), inFp);
    fgets(comment, sizeof(comment), inFp);




    // if compatibility mode is turned on, this module creates additional empty macro
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

        // end of section
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


        if(port < 0 )
        {
            message(ERR, "Bad PORT: %d\n", port);
            return -1;
        }


        if(pin < 0 )
        {
            message(ERR, "Bad PIN: %c\n", pin);
            return -1;
        }






        //creating set of macros for 1 gpio
        if(lpc17xx_printMacro(outFp, mode[0], port, pin, name, comment) != 0)
            return -1;


    } while(1);

    return macrosNum;
}




/*---------------------------------------------------*/


//write set of macros for one pin
static int lpc17xx_printMacro(FILE* outFp, const char mode, unsigned int port, unsigned int pin, const char* name, const char* comment)
{

    switch(mode)
    {
        case 'i':   //Digital input

            fprintf(outFp, "/* %s - P%d[%d] - digital input \n\t %s */\n\n", name, port, pin, comment);

            fprintf(outFp, "%s" "%s_dirIn" "%s" "/*instr;*/" "%s", macroFmt->mBegin, name, macroFmt->mMid, macroFmt->mEnd);


            // fprintf(outFp, "#define %s_isHigh()     ((LPC_GPIO%d->DATA & (1<<%d)) != 0)\n\n", name, port, pin);

            // fprintf(outFp, "#define %s_isLow()      ((LPC_GPIO%d->DATA & (1<<%d)) == 0)\n\n", name, port, pin);

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


//some informations
void lpc17xx_help(void)
{
    printf("NXP LPC175x & LPC176x series ( ARM Cortex M3 core )                     \n"
           "                                                                        \n"
           "This module doesn't check if you use valid pin                          \n"
           "- it must be checked in part datasheet / user manual.                   \n"
           "                                                                        \n"
           "LPC177x & 178x series are not supported.                                \n"
           "If you are interested, it's possible to easy add new module for m-gen.  \n"
           "See: https://github.com/Leopardus4/m-gen/blob/master/CONTRIBUTORS.md    \n"
           "                                                                        \n"
           );
}