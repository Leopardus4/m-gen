#ifndef GM_UTILS_H
#define GM_UTILS_H

/*
File:       gm-utils.h
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



/*
Function copies strFrom to strTo and adds or changes extension
    to newExt (in format ".new" - with or without dot).
size - size of strTo buffer.
If strTo is too short, name from strFrom will be truncated to fit newExt.
*/
void changeExtension(char* strTo, const char* strFrom, size_t size, const char* newExt);


/*
This function tests if given file exist in working directory
Returns 1 if yes
    or 0 if no.
*/
int fileExist(const char* filename);



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
void createHeaderGuard(char* strTo, const char* strFrom, size_t size);



/*
Funcion finds given section (sectionLetter) (in format $x)
    in given file (read permission required),
    sets position in file to start of this sections (after '$' and sectionLetter)
    and additionally returns this position
        or -1 in case of error.
*/
long findSection(FILE* fp, char sectionLetter);



/*
Printing informations about macros
*/
void printPinModes(FILE* output);
void printPinMacros(FILE* output);



#endif // GM_UTILS_H
