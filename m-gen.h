#ifndef GM_H
#define GM_H

/*
File:       m-gen.h
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

#define VERSION "1.0"


/* boolean : true / false */
#if __STDC_VERSION__ >= 199901L
    #include <stdbool.h>  /* new data type: 'bool' and values: 'true' and 'false' - added in C99 */
#else
    typedef enum {false=0, true=(!false)} bool;
#endif // __STDC_VERSION__




/* indexing targets */
#define HOW_MANY_TARGETS    2      //number of supported targets

#define _AVR    0
#define _LPC111X    ( ( _AVR ) + 1 )


/* storing names of targets
*/
typedef enum{
    ANY     = -1,
    AVR     = _AVR,
    LPC111X = _LPC111X
} TARGETS;


#define FILENAME_LENGTH (256)


/*
Flags for target module - for future use
*/
typedef struct{

    bool thisFlagDoesNothing;

} TARGET_FLAGS;

/*
This structure contains all flags & options
*/
typedef struct{

    bool help;
    bool showVersion;
    bool init;
    bool otherName;

    TARGET_FLAGS targetFlags;

    TARGETS target;

    char inputFileName[FILENAME_LENGTH];
    char outputFileName[FILENAME_LENGTH];
} FLAGS;




#define DESCRIPTION_LENGTH 256

/*
This structure contains attributes and pointers to functions for one specific target
*/
typedef struct{

        //some informations about target
    char description[DESCRIPTION_LENGTH];

        // functions pointers
    void    (*init)     (FILE* fp, const TARGET_FLAGS* fls);
    int     (*macroGen) (FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls);
    void    (*help)     (void);

} TARGET_ATTRIBUTES;



#define TARGET_NAME_LENGTH   32

/*
    structure - label to target
*/

typedef struct{

char name[TARGET_NAME_LENGTH];  //short, only lowercase, digits and '-' (instead of '_' and whitespaces)

void (*getData)(TARGET_ATTRIBUTES* atrs);    //pointerto target's initializer

} TARGET_LABEL;


/*
Print message to output
    level - predefined macro - see below
    format, ... - printf() syntax

*/
void message(int level, const char* format, ...);

/*
Macros for message() - to categorize message value

MSG - printed to stdout
other - stderr
*/
#define MSG     0   /* simple message */
#define NOTE    1   /* important informations about converting macros */
#define WARN    2   /* warning - it should not stopped generating macros */
#define ERR     3   /* error - should be followed by program exit */
#define FATAL   4   /* used only if program is not complete - i. e. uninitialized function pointers */


// Prefixes for messages - message() adds it automatically
#define P_MSG       ""  /* nothing */
#define P_NOTE      "Note:"
#define P_WARN      "Warning:"
#define P_ERR       "Error:"
#define P_FATAL     "FATAL ERROR:"


/*

*/
#define ERR_MSG_INCOMPLETE_SOURCES  "Program was compiled from incomplete sources\n"\
"\tPlease download latest stable release\n"


/*  (only) for developers, this macro shows which functions were called    */
//#define FUNCINFO()    printf("\t-+- %s() function\n", __func__)
/*  When unused, comment it and uncomment following:    */
#define FUNCINFO()



/*
for slow program ha ha
*/
#ifdef __WIN32
  #include <windows.h>

  #define sleep(seconds) Sleep(1000 * (seconds))

#elif defined __unix__
  #include <unistd.h>

#else
  #define sleep(seconds)
        //do nothing

#endif // __WIN32 / __unix__




#endif // GM_H
