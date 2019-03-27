#ifndef LPC111X_H
#define LPC111X_H

/*
File:		lpc111x.h
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





/*---------------------------------------------------*/

/* functions declarations */

void lpc111x_getData(TARGET_ATTRIBUTES* atrs);

void lpc111x_init(FILE* fp, const TARGET_FLAGS* fls);

int  lpc111x_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls);

void lpc111x_help(void);


/*---------------------------------------------------*/

#endif // LPC111X_H
