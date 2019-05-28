/*
File:       gm-utils.c
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
#include <string.h> //strcmp(), strrchr()
#include <ctype.h>  //toupper()

#include "gm-utils.h"
#include "m-gen.h"



/*---------------------------------------------------*/

/*
Function copies strFrom to strTo and adds or changes extension
    to newExt (in format ".new" - with or without dot).
size - size of strTo buffer.
If strTo is too short, name from strFrom will be truncated to fit newExt.
*/

void changeExtension(char* strTo, const char* strFrom, size_t size, const char* newExt)
{
    char* extension;
    int position;
    const int newExtLenght = strlen(newExt) + 1;

    extension = strrchr(strFrom, '.');


    if(extension == NULL)
    {
        position = strlen(strFrom);

        if(position <= (size - newExtLenght) )
            snprintf(strTo, size, "%s%s", strFrom, newExt);

        else
        {
            snprintf(strTo, size, "%s", strFrom);

            snprintf( &(strTo[size-newExtLenght]), newExtLenght, "%s", newExt);
        }
    }

    else
    {
        snprintf(strTo, size, "%s", strFrom);

        position = (int) (extension - strFrom);


        if(position <= (size - newExtLenght) )
            snprintf( &(strTo[position]), size - position, "%s", newExt);

        else
            snprintf( &(strTo[size-newExtLenght]), newExtLenght, "%s", newExt);
    }
}




/*---------------------------------------------------*/

/*
This function tests if given file exist in working directory
Returns 1 if yes
    or 0 if no.
*/
int fileExist(const char* filename)
{
    FILE* fp = fopen(filename, "r");

    if(fp != NULL)
    {
        fclose(fp);
        return 1;
    }

    else
        return 0;
}




/*---------------------------------------------------*/

/*
Function creates 'Header guard' from given filename.

All lower case will be replaced by upper case,
    digits and upper case will be copied,
    other characters will be replaced by underscore character.

Function will end:
    if strFrom[i] == '\0'       ---'zero' will be copied

    or if strFrom[size-1] != 0  ---strFrom[size-1] will be ignored
                                 and strTo[size-1] will get '\0' value.
*/
void createHeaderGuard(char* strTo, const char* strFrom, size_t size)
{
    for(int i=0; i<(size-1); ++i)
    {

        if(isalnum(strFrom[i]))
        {
            if(islower(strFrom[i]))
                strTo[i] = toupper( (unsigned)strFrom[i] );

            else
                strTo[i] = strFrom[i];

        }

        else if(strFrom[i] == '\0')
        {
            strTo[i] = '\0';
            return;
        }


        else
            strTo[i] = '_';
    }

    strTo[size-1] = '\0';
}



/*---------------------------------------------------*/

/*
Funcion finds given section (sectionLetter) (in format $x)
    in given file (read permission required),
    sets position in file to start of this sections (after '$' and sectionLetter)
    and additionally returns this position
        or -1 in case of error.
*/
long findSection(FILE* fp, char sectionLetter)
{
    char c;

    if(sectionLetter == '$')
        return -1;

    // goto start of file
    if( fseek(fp, 0, SEEK_SET) != 0)
        return -1;

    while( (c=getc(fp)) != EOF )
    {
        if(c == '$')    //'dollar' - new section sign
        {
            c = getc(fp); //next character
            if(c == sectionLetter)
                return ftell(fp);
        }
    }

    message(ERR, "Cannot find section '$%c' in input file. \n", sectionLetter);
    return -1;
}


/*---------------------------------------------------*/

void printPinModes(FILE* output)
{
    fprintf(output,
        "Avaiable modes of GPIO:                                            \n"
        "     digital gpio - i. e. communication with other digital chips   \n"
        "   i   digital Input                                               \n"
        "   o   digital Output                                              \n"
        "   d   Double (in+out)                                             \n"
        "                                                                   \n"
        "     other modes:                                                  \n"
        "   b   Button              - for input buttons / sensors           \n"
        "          (it uses internal pull-up resistor)                      \n"
        "   l   active Low output   -  / for transistors / leds etc.        \n"
        "   h   active High output  - /                                     \n"
        "                                                                   \n"
        );
}


/*---------------------------------------------------*/



void printPinMacros(FILE* output)
{
    fprintf(output,
        "m-gen creates set of macros for each gpio.                 \n"
        "It depends on gpio mode from .gm file.                     \n"
        "                                                           \n"
        "All macros which should only set something in registers    \n"
        "are equivalents of void xxxx(void) functions,              \n"
        "or in 'inline' mode, are declared as such.                 \n"
        "                                                           \n"
        "Macros created to test something (abc_isActive(), ...)     \n"
        "returns zero (false) or non-zero value (true),             \n"
        "so may be used in conditionals (if-else, \"?:\", ...)      \n"
        "If 'inline' mode is used, they are declared as             \n"
        "uintX_t xxxx(void), where \"X\" is the fastest length for  \n"
        "given CPU (i. e. 32 for Cortex M).                         \n"
        "                                                           \n"
        "                                                           \n"
        "                                                           \n"
        " [ #define gpio_enableAccess() ]                           \n"
        "       This macro is present only on some platforms.       \n"
        "       If present, it's used to i. e. turn on gpio clock   \n"
        "       (it's common for all gpio,                          \n"
        "        and MUST be used before any other macros)          \n"
        "                                                           \n"
        "                                                           \n"
        "                                                           \n"
        "Macros for 1 pin:  (abc - symbolic pin name from .gm file) \n"
        "                                                           \n"
        "digital Input (i):                                         \n"
        "                                                           \n"
        "   #define abc_dirIn()     - initializing gpio as input    \n"
        "                                                           \n"
        "   #define abc_isHigh()    + testing state of gpio         \n"
        "                           | i. e.:                        \n"
        "   #define abc_isLow()     + if( abc_isHigh() ) {code}     \n"
        "                                                           \n"
        "                                                           \n"
        "                                                           \n"
        "digital Output (o):                                        \n"
        "                                                           \n"
        "   #define abc_dirOut()    - initializing gpio as output   \n"
        "                                                           \n"
        "   #define abc_setHigh()   + changing state of gpio        \n"
        "                           | (high/low)                    \n"
        "   #define abc_setLow()    +                               \n"
        "                                                           \n"
        "       It's required to use setHigh() or setLow() macro    \n"
        "       after dirOut(), because state of gpio is unknown.   \n"
        "                                                           \n"
        "                                                           \n"
        "                                                           \n"
        "digital input and output (d):                              \n"
        "                                                           \n"
        " [ #define abc_init() ]    - present only on some platforms\n"
        "       It initializes pin as gpio                          \n"
        "       (and i. e. disables pull-up)                        \n"
        "       If present, should be used before any operations    \n"
        "       on this gpio.                                       \n"
        "       It's required to use dirIn() or dirOut()            \n"
        "       after init(), because direction is unknown.         \n"
        "                                                           \n"
        "   #define abc_dirIn()     +                               \n"
        "   #define abc_dirOut()    |   As described above          \n"
        "                           |                               \n"
        "   #define abc_isHigh()    |                               \n"
        "   #define abc_isLow()     |                               \n"
        "                           |                               \n"
        "   #define abc_setHigh()   |                               \n"
        "   #define abc_setLow()    +                               \n"
        "                                                           \n"
        "                                                           \n"
        "                                                           \n"
        "active low / active high output (l/h):                     \n"
        "  (macros for both are identicall,                         \n"
        "   so it's possible to exchange them)                      \n"
        "                                                           \n"
        "   #define abc_asOutput()  - initializing gpio as output   \n"
        "                             and changing state to 'Off'   \n"
        "                                                           \n"
        "   #define abc_On()        - changing state of gpio        \n"
        "                             to turn 'On' actuator         \n"
        "                                                           \n"
        "   #define abc_Off()       - changing state of gpio        \n"
        "                             to turn 'Off' actuator        \n"
        "                                                           \n"
        "                                                           \n"
        "                                                           \n"
        "active low input with internal pull-up resistor (b):       \n"
        "                                                           \n"
        "   #define abc_asInput()   - initializing gpio as input    \n"
        "                             and turning on pull-up        \n"
        "                                                           \n"
        "   #define abc_isActive()  - checking if sensor is active  \n"
        "                             i. e.:                        \n"
        "                             if( abc_isActive() ) {code}   \n"
        "                                                           \n"
        "   #define abc_isInactive()- checking if sensor is inactive\n"
        "                                                           \n"
        "                                                           \n"
        );

}


