/*
File:       gm-common.c
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
#include <ctype.h>  //toupper()

#include "gm-common.h"


/*---------------------------------------------------*/

int getActualLine(FILE* fp)
{
    int line = 1;

    long position = ftell(fp);


    fseek(fp, 0, 0);

    while( ftell(fp) < position)
    {
        if(fgetc(fp) == '\n')
            ++line;
    }

    line -=1;   // why?

    return line;
}




/*---------------------------------------------------*/



void strToUpper(char* str)
{
    if(str == NULL)
        return;

    for(int i=0; str[i]; ++i)
    {
        str[i] = toupper(str[i]);
    }
}



void strToLower(char* str)
{
    if(str == NULL)
        return;

    for(int i=0; str[i]; ++i)
    {
        str[i] = tolower(str[i]);
    }
}


