/*
File:       m-gen.c
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
#include <string.h> //strcmp()
#include <stdarg.h>


#include "m-gen.h"
#include "gm-utils.h"
#include "gm-common.h"

// targets:
#include "avr.h"
#include "lpc111x.h"





/* Function declarations */

int readParameters  (int argc, char * argv [], FLAGS* fls, TARGET_LABEL labels[]);

int createInputFile(const FLAGS* fls, const TARGET_ATTRIBUTES* atr, const char* targetname);

int generateMacros(FLAGS* fls, const TARGET_LABEL labels[]);


void help(TARGET_LABEL labels[]);

void printMainPage(void);
void printVersion(void);
void printProgHeader(void);



/* static variables */

static int silentLevel;







int main(int argc, char * argv [])
{
    FUNCINFO();


    int ret_val=0;


    // pointers to initializers
    TARGET_LABEL labels[HOW_MANY_TARGETS] = {

        //AVR
        [_AVR] = {
            .name = "avr",
            .getData = &avr_getData
        },

        [_LPC111X] = {
            .name = "lpc111x",
            .getData = &lpc111x_getData
        }

        // next targets should be initialized here

    };
    /*
    This array contains references to supported targets.
    All operations on it must use following syntax:
    labels[_TARGETMACRO] or labels[TARGETS_enumeration]
    _TARGETMACRO is defined in gm.h file as a constans number.
    enumeration TARGETS - i. e. flags.target - see below.
    */



    //structure initialized by labels[x].getData()
    TARGET_ATTRIBUTES targetAttrs = {0};


    //flags given as parameters
    FLAGS flags={

        .help = false,
        .showVersion = false,
        .init = false,
        .otherName = false,

        .target = ANY,

        .inputFileName[0] = 0,
        .outputFileName[0] = 0,
    };


    if(argc <= 1)
    {
        printMainPage();
        return 0;
    }


    ret_val = readParameters(argc, argv, &flags, labels);

    if(ret_val != 0)
        return ret_val;



    printProgHeader();


    if(flags.showVersion==true)
    {
        printVersion();
        return 0;
    }


    //initializing targetAttrs structure
    if(flags.target != ANY)
    {
        if( labels[flags.target].getData == NULL)
        {
            message(FATAL, "%s\n", ERR_MSG_INCOMPLETE_SOURCES);
            return 101;
        }


        labels[flags.target].getData(&targetAttrs);

        if(targetAttrs.help == NULL
           || targetAttrs.init == NULL
           || targetAttrs.macroGen == NULL )
        {
            message(FATAL, "%s\n", ERR_MSG_INCOMPLETE_SOURCES);
            return 101;
        }
    }


    if(flags.help==true)
    {
        if(flags.target==ANY)
            help(labels);
        else
            targetAttrs.help();


        return 0;
    }



    else if(flags.init==true)
    {
        if (flags.target != ANY)
            return createInputFile(&flags, &targetAttrs, labels[flags.target].name);
        else
        {
            fprintf(stderr, "Error: Target not specified\n");
            return 1;
            /* if type of target is not given as parameter, program will return error */
        }
    }

    else
        return generateMacros(&flags, labels);
}








/*---------------------------------------------------*/

/*---------------------------------------------------*/

/* Reading arguments */
int readParameters( int argc, char * argv [], FLAGS* fls, TARGET_LABEL labels[] )
{
    FUNCINFO();


    for(int i=1; i<argc; ++i)
    {
        if( (strcmp(argv[i], "-h")==0)
              || (strcmp(argv[i], "--help")==0) )
            fls->help = true;

        else if( (strcmp(argv[i], "-v")==0)
              || (strcmp(argv[i], "--version")==0) )
            fls->showVersion = true;


        // Creating input file (.gm)
        // If this parametr is not given, program will create '.h' file with macros from given file
        else if( (strcmp(argv[i], "-i")==0)
              || (strcmp(argv[i], "--init")==0) )
            fls->init = true;

        else if(strcmp(argv[i], "-o")==0)
        {
            fls->otherName = true;
            strncpy(fls->outputFileName, argv[++i], (FILENAME_LENGTH - 1) );    //next argument - output file name
            fls->outputFileName[FILENAME_LENGTH - 1]=0;
        }

        // 'target' parameter
        else if(strcmp(argv[i], "-t")==0)
        {

            if(fls->target != ANY)
            {
                fprintf(stderr, "Error: target redefined\n");
                return 1;
            }

            ++i;    //next - target name

            for(int j=0; j<HOW_MANY_TARGETS; ++j)
            {
                if(strcmp(argv[i], labels[j].name) ==0)
                    fls->target = j;
            }

            if(fls->target == ANY)
            {
                fprintf(stderr, "Error: unknown target: %s\n", argv[i]);
                return 1;
            }
        }

        //Here insert new supported parameters
        // ...


        // Wrong parameter
        else if(argv[i][0] == '-')
            {
                fprintf(stderr, "Error: unknown parameter: %s\n", argv[i]);
                return 1;
            }


        // If actual parameter is not a flag (first character !=  '-' ),
        // it's a name of input file.
        else
        {
            if(fls->inputFileName[0] == 0)
                strncpy(fls->inputFileName, argv[i], (FILENAME_LENGTH - 1) );
            else
            {
                fprintf(stderr, "Error: too many input files given: %s and %s\n", fls->inputFileName, argv[i]);
                return 1;
            }
        }

    }

    return 0;
}


/*---------------------------------------------------*/
/*
This function creates an input file (.gm)
*/

int createInputFile(const FLAGS* fls, const TARGET_ATTRIBUTES* atr, const char* targetname)
{
    FUNCINFO();

    printf("Generating file %s\n"
           "\tTarget: %s\n", fls->inputFileName, targetname);



    if(fls->inputFileName[0] == 0)
    {
        fprintf(stderr, "Error: please put file name\n");
        return 1;
    }


    FILE* fp;

    /* If file already exist, return error */
    if(fileExist(fls->inputFileName))
    {
        fprintf(stderr, "Error: file '%s' already exist,please change filename\n"
            /* "    or use '-f' option (actually not supported)\n" */
            , fls->inputFileName);

        return 1;
    }



    /* Creating skelet of file */

    fp = fopen(fls->inputFileName, "w");

    if(fp == NULL)
    {
        perror(fls->inputFileName);
        return 1;
    }


    fprintf(fp, "File auto-generated by m-gen v%s program. \n"
    			"	(see: https://github.com/Leopardus4/m-gen ) \n"
         		"Please write some macro prototypes and use m-gen to create C header file from it.\n"
         		"\n", VERSION);


    /*
    All sections in input file starts at sequence '$x'
    where 'x' is reserved letter for definite section
    */

    /*
    Printing target name in format:

        $t
        TARGET

    */
    fprintf(fp, "$t" "\n");
    fprintf(fp, "%s\n\n", targetname);

    /*
    Printing comment about file in format:

        $c
        Example
        long
        comment

    */
    fprintf(fp, "$c" "\n");
    fprintf(fp, "Write here your comment"
         "\n\t about whole file\n\n");


    /*
    Creating macros section and 'other' section.
    This fragment calls proper function (via pointer from TARGET_ATTRIBUTES structure).

    Called function must write following sections:

    (example for AVR)

        $m
        Mode PORT PIN Comment

        i   B   4   Exemplary comment...

        $o
        Some info how to use this sheet (avaiable modes etc... )

    */

    atr->init(fp, &(fls->targetFlags) );


    /*
    additional informations about *.gm file (at the end of '$o' section)
    */
    fprintf(fp,
        "                                                                   \n"
        "Write some macros and use 'm-gen filename' command                 \n"
        "   to create C header file with '#define' macros.                  \n"
        "Use 'm-gen --help' for more info.                                  \n"
        "                                                                   \n"
        "-------------------------------------------------------------------\n"
        "                                                                   \n"
        "Do not use single 'dollar' character ($$) in this file             \n"
        "   ( if it's necessary, use two $$ ).                              \n"
        "Do not use '*/' sequence in comments                               \n"
        "   (comments will be automaticcaly placed in /* ... */ )           \n"
        "Do not edit '$$t' section.                                         \n"
        "                                                                   \n"
        );


    fclose(fp);

    printf("Done\n");

    return 0;
}




/*---------------------------------------------------*/


int generateMacros(FLAGS* fls, const TARGET_LABEL labels[])
{
    char tempFile[] = ".g-men_tmp.tmp";

    char headerGuard[FILENAME_LENGTH] = {0};

    int retval = 0;

    int macrosNum;

    TARGET_ATTRIBUTES attrs = {0};



    if(fls->inputFileName[0] == 0)
    {
        fprintf(stderr, "Error: please put input file name\n");
        return 1;
    }


    /*
        Changing 'filename.gm' to 'filename.h' (default).
    */

    if(fls->otherName == false)
        changeExtension(fls->outputFileName, fls->inputFileName, FILENAME_LENGTH, ".h");



    printf("Generating file %s  from  %s\n", fls->outputFileName, fls->inputFileName);



    /*
        Input file - *.gm
    */
    FILE* inFp = fopen(fls-> inputFileName, "r");


    if(inFp == NULL)
    {
        perror(fls->inputFileName);
        return 1;
    }





    /*
        Output file - temporary
    */

    if(fileExist(tempFile))
        remove(tempFile);


    FILE* outFp = fopen(tempFile, "w");



    /*
        Reading targetname
    */

    {
        char targetName[TARGET_NAME_LENGTH] = {0};
        int i;


        if(fls->target != ANY)
        {
            fprintf(stderr, "Error: Don't use '-t' option\n"
                "   ('target' will be read from .gm file).\n");

            retval = 1;
            goto close_fs;
        }


        if( findSection(inFp, 't') < 0) // go to start of '$t' section in .gm file
        {
            retval = 1;
            goto close_fs;
        }

        fscanf(inFp, " %31s", targetName);


        for(i=0; i<HOW_MANY_TARGETS; ++i)
        {
            if(strcmp(targetName, labels[i].name)==0)
                fls->target = i;
        }


        if(fls->target == ANY)
        {
            fprintf(stderr, "Error: unknown target '%s'\n", targetName);

            retval = 1;
            goto close_fs;
        }
    }


    printf("\tTarget: %s\n", labels[fls->target].name);


    //initilalizing attrs structure
    if(labels[fls->target].getData == NULL)
    {
        message(FATAL, "%s\n", ERR_MSG_INCOMPLETE_SOURCES);

        retval = 1;
        goto close_fs;
    }


    labels[fls->target].getData(&attrs);


    if(attrs.macroGen == NULL)
    {
        message(FATAL, "%s\n", ERR_MSG_INCOMPLETE_SOURCES);

        retval = 1;
        goto close_fs;
    }




    /*
        Creating content of .h file.
    */



    /*
        #ifndef NAME_H
        #define NAME_H


        ... at the end
        #endif // NAME_H
    */

    createHeaderGuard(headerGuard, fls->outputFileName, FILENAME_LENGTH);

    fprintf(outFp, "#ifndef %s\n", headerGuard);
    fprintf(outFp, "#define %s\n\n", headerGuard);


    /*
        "extern C" for C++
        (for macros it doesn't matter, but for future implementation of functions ... )
    */

    fprintf(outFp, "#ifdef __cplusplus\n"
                    "  extern \"C\" {\n"
                    "#endif\n\n" );


    /*
        Preface
    */
    fprintf(outFp,
         "/*\n"
         "File auto-generated by m-gen v%s                  \n"
         "   (see https://github.com/Leopardus4/m-gen )     \n"
         "\n"
         "DO NOT EDIT THIS FILE!                            \n"
         "Please edit apprioritate .gm file and run m-gen   \n"
         "\n"
         "*/\n\n", VERSION);




    /*
        User's comment from .gm file
    */

    fprintf(outFp, "/*\n");

    {
        char c;

        if(findSection(inFp, 'c') < 0)
        {
            retval = 1;
            goto close_fs;
        }

        do  //loop breaks at the end of section, but ignores every two "$$"
        {
            while( (c = fgetc(inFp)) != '$')
                {
                    if(c == EOF)
                        goto endComment;
                    fputc(c, outFp);
                }

        } while((c = fgetc(inFp)) == '$');

    }

    endComment:

    fprintf(outFp, "*/\n");



    fprintf(outFp, "\n\n\n\n//------------------------------------------------------------------------//\n\n");


    /*
        Macros and description about usage
        - proper function is called.
    */

    if(findSection(inFp, 'm') < 0)  //go to '$m' section
    {
        retval = 1;
        goto close_fs;
    }


    macrosNum = attrs.macroGen(inFp, outFp, &(fls->targetFlags));

    if(macrosNum < 0)
    {
        fprintf(stderr, "\t(line: %d )\n", getActualLine(inFp));
        retval = 1;
        goto close_fs;
    }





    /*
        end of ' extern "C" '
    */

    fprintf(outFp,  "\n"
                    "#ifdef __cplusplus\n"
                    "  }\n"
                    "#endif\n\n" );



    /*
        #endif of Header guard
    */
    fprintf(outFp, "#endif    // %s\n", headerGuard);



    close_fs:

    fclose(outFp);
    fclose(inFp);


    /*
    From tmp file to output file ...
    */

    if(retval != 0)
    {
        remove(tempFile);
        return 1;
    }

    else
    {
        if(fileExist(fls->outputFileName))
        {
            char prevFile[FILENAME_LENGTH];

            changeExtension(prevFile, fls->outputFileName, FILENAME_LENGTH, "_prev.h.txt");

            if(fileExist(prevFile))
                remove(prevFile);

            rename(fls->outputFileName, prevFile);
        }

    rename(tempFile, fls->outputFileName);

    }


    // ha ha ha
    for(int i=0; i<3; ++i)
    {
        printf(".");
        fflush(stdout);
        sleep(1);
    }


    printf(" Done. Macros for %d pins written.\n", macrosNum);



    return 0;
}



/*---------------------------------------------------*/

/*---------------------------------------------------*/

/*
Print message to output
    level - predefined macro from m-gen.h (i. e. MSG or ERR )
    format, ... - printf() syntax

*/
void message(int level, const char* format, ...)
{
    va_list args;


    const char* prefix[5];

    prefix[MSG]     = P_MSG;
    prefix[NOTE]    = P_NOTE;
    prefix[WARN]    = P_WARN;
    prefix[ERR]     = P_ERR;
    prefix[FATAL]   = P_FATAL;

    FILE* output = (level == MSG) ? stdout : stderr;



    if(silentLevel <= level)
    {
        va_start(args, format);


        fprintf(output, "%s ", prefix[level]);

        vfprintf(output, format, args);

        fflush(output);


        va_end(args);
    }


}

/*---------------------------------------------------*/

/*---------------------------------------------------*/


void printMainPage(void)
{
    printf( "m-gen\n"
            "   Macro GENerator\n"
            "\n"
            "Use --help or --version parameter for more informations\n");
}


/*---------------------------------------------------*/

void help(TARGET_LABEL labels[])
{
    printf ("Usage:\n"
            "    m-gen file.gm --init -t target \n"
            "to create empty array in file.gm \n"
            " \n"
            "You should write some macro prototypes to this file \n"
            " \n"
            "Then use: \n"
            "    m-gen file.gm [ -o other_name.h ] \n"
            "to convert it to macros in new .h file. \n"
            " \n"
            " [...] - optional \n"
            " \n"
            "Options: \n"
            " \n"
            "   -v  (or '--version')  show version of program \n"
            " \n"
            "   <-t target>           select target (MCU series). See below for available targets. \n"
            " \n"
            "   -h  (--help)          this page. Usage <-h -t target> shows help for \"target\" \n"
            " \n"
            "   -i  (--init)          creates input file (.gm). If this parameter is not given, \n"
            "                           m-gen will convert .gm file to .h file with macros \n"
            " \n"
            "   <-o othername>        output file will get name \"othername\". \n"
            "                           When unused, output file will be named \"input_filename.h\". \n"
            " \n");

    {
        TARGET_ATTRIBUTES attrs = {0};

        printf ("Targets: \n\n");

        for(int i=0; i<HOW_MANY_TARGETS; ++i)
        {
            labels[i].getData(&attrs);

            printf("%s\t%s\n\n", labels[i].name, attrs.description);
        }
    }
}


/*---------------------------------------------------*/

void printVersion(void)
{

    // printProgHeader() is called before printVersion(), so there is no need to print "m-gen vX.XX"

    printf("(c) 2019 leopardus \n"
    " \n"
    "This program is distributed under GPLv3 General Public License. \n"
    "You should received copy of this license. In other case, visit \n"
    "    www.gnu.org/licenses \n"
    " \n"
    "THIS IS A FREE SOFTWARE. IT'S PROVIDED \"AS IS\", WITHOUT ANY WARRANTIES. \n"
    " \n"
    "You can find source code here: \n"
    "    https://github.com/Leopardus4/m-gen \n"
    " \n"
    "\033[107m       \033[0m\n"
    "\033[101m       \033[0m\n\n");

}


/*---------------------------------------------------*/

void printProgHeader(void)
{
    printf("\nm-gen v%s\n\n", VERSION);
}
