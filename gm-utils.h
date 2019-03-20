#ifndef GM_UTILS_H
#define GM_UTILS_H

/*
File:		gm-utils.h
Project:	m-gen
Version:	1.0

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


void changeExtension(char* strTo, const char* strFrom, size_t size, const char* newExt);

int fileExist(const char* filename);

void createHeaderGuard(char* strTo, const char* strFrom, size_t size);

long findSection(FILE* fp, char sectionLetter);




#endif // GM_UTILS_H
