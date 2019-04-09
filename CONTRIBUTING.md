m-gen
=====


All code is distributed under GPL license (see LICENSE.txt),
    so you can do (almost) everything with it.

__But__ if you want to add your improvement to official release:


- if you want to develop primary parts of program
        (i. e. your own idea / or there are a couple of [TODO-s](https://github.com/Leopardus4/m-gen/blob/master/TODO.md))
        please contact me via GitHub or mail ( leopardus3[a]pm.me - please add a tag [m-gen] in title )

- or if you want to add support for new device (TARGET),
        read following description.
        In mail with me, please add a tag "[m-gen ~ NEW TARGET]" in the topic.


---

How to add new target:
======================

- create new files: _file.h_ and _file.c_ in _targets/_ directory (replace filename with short name of target)



## file .h:


---

    #ifndef NAME_H
    #define NAME_H
    
    void myTarget_getData(TARGET_ATTRIBUTES* atrs);

    void myTarget_init(FILE* fp, const TARGET_FLAGS* fls);

    int  myTarget_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls);

    void myTarget_help(void);

    #endif // NAME_H

---



## file .c :

    #include <stdio.h> //and other needed headers
    
    #include "m-gen.h"
    #include "gm-common.h"  //if needed
    
    #include "my_new_header.h"



- implementation functions from .h: (you can see other source codes in targets/ directory)



    - myTarget_getData(TARGET_ATTRIBUTES* atrs) - should returns pointers to other functions:

            {
            char desc[DESCRIPTION_LENGTH] = "Short info about my target";
        
            strncpy(atrs->description, desc, DESCRIPTION_LENGTH);
        
            atrs->help      =  &myTarget_help;
            atrs->init      =  &myTarget_init;
            atrs->macroGen  =  &myTarget_generateMacros;
            
            // modes supported by module - see below
            atrs->presentModes.modeMode1 = true;    // example
            }


    - myTarget_init(FILE* fp, const TARGET_FLAGS* fls)

        - FILE* fp - pointer to input file with write permission
        
        - const TARGET_FLAGS* fls - pointer to structure with flags - modes
        
        - this function should write sth like this to given file:
        
                $m
                Mode PORT PIN Name Comment
                
                l	B	4	led_status	Write here your own macros...
                
                $o
                description about chip-dependent columns (port, pin)
               

    - myTarget_generateMacros(FILE* inFp, FILE* outFp, const TARGET_FLAGS* fls)
        
        - FILE* inFp - file created by init() but only with read permission.
            (position is set AFTER '$m" but BEFORE '\n' character)
       
        - FILE* outFp - output file (.h) - function should write macros to this file
        
        - const TARGET_FLAGS* fls - as in init()
        
        - returned value:
            -  _-1_ in case of error (function should print info about error)
            
            -  or nonnegative number of pins for whose function generated macros
        
        - this function should read data from input file, line by line, and convert it into macros in output file.
            Macros should use IDENTICALL syntax for all targets - see 'm-gen --help'. If it's impossible, please contact me. 
            End of array in input file is signed as _$_ character at beginning of line (probably _$o_ - beginning of "other" section.



    - myTarget_help()
        
        - some info about _target_


- structure _TARGET_FLAGS_ contains operating modes.
    - Inside getData() (as _presentModes_ nested struture) it's used to estabilish if module supports one of them.
    - Inside generateMacros() or init() (as *fls) it should be used to check if specific mode is selected by end-user, and if yes, do something...
    
    If your module supports one of them, it should be set inside getData(TARGET_ATTRIBUTES* atrs) to _true_,
    and implemented inside generateMacros() or init() (inside _if(fls->modeMode1==true)_ statements).
    
    m-gen at beginning will check if specific mode is called by user (as command line argument), will check if your module supports it
    (in other case will print proper information), and will set corresponding flag before calling generateMacros()
    
    If your module doesn't support specific mode, take it easy - atrs.presentModes struct is initialized by {0} before calling getData(),
    so main program will know that your module doesn't support this mode.
    
    Present modes:
    
    - _compatibilityMode_ - creating full set of macros (if it's no need to use specific macro, it should be created as empty macro)
        See _m-gen --help_ for details.
    




- all functions should use message(level, format, ...) function instead of printf(format, ...).

    _level_ - use one of predefined macros from _m-gen.h_ : _MSG, NOTE, WARN, ERR_ - ERR should be followed by _return -1;_

    Be careful - message() uses va_list - if arguments list is not compatible with _format_, at run time it may create "memory protection violation".


- you can create other functions if needed, but their declarations should be placed __only__ in your .c file, with _static_ keyword.

---

## in file gm.h:

- change *HOW_MANY_TARGETS* definition to right value

- add definition:

        #define _TARGET    ( ( _PREVIOUSTARGET ) + 1 )

- add new member to *TARGETS* enumeration:

        TARGET = _TARGET

(don't forget about comma after previous element)


---

## in file gm.c:

    #include "my_file.h"

#### in main():

- initialize new element of labels[] array:


        [_TARGET]={
            .name = "my-target",    //short, without spaces, use '-' instead of '_'
            .getData = &myTarget_getData
        }
    
    (don't forget about comma after previous element)



---

__That's all. Simple, dunnit?__

