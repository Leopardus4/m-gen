m-gen
=====


All code is distributed under GPL license (see LICENSE.txt),
    so you can do (almost) everything with it.

__But__ if you want to add your improvement to official release:

- if you want to develop primary parts of program,
        please contact me ( leopardus3[a]pm.me )

- or if you want to add support for new device (TARGET),
        read following description.
        In mail with me, please add a tag "[NEW TARGET]" in the topic.


---
How to add new target:
======================

- create new files: _file.h_ and _file.c_ (replace name with short name of target)



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

         #include <stdio.h> //and other
         
         #include "m-gen.h"
         #include "gm-utils.h"  //if needed
         #include "my_new_header.h"



- implementation functions from .h: (you can see other source codes in targets/ directory)



    - myTarget_getData() - should returns pointers to other functions:

            {
            char desc[DESCRIPTION_LENGTH] = "about my target";
        
            strncpy(atrs->description, desc, DESCRIPTION_LENGTH);
        
            atrs->help      =  &myTarget_help;
            atrs->init      =  &myTarget_init;
            atrs->macroGen  =  &myTarget_generateMacros;
            }


    - myTarget_init()
        - FILE* fp - pointer to input file with write permission
        
        - const TARGET_FLAGS* fls - pointer to structure with flags - actually empty - only for future compatibility
        
        - this function should write sth like this to given file:
        
                $m
                Mode PORT PIN Name Comment
                
                l	B	4	led_status	Write here your own macros...
                
                $o
                description about usage!
               

    - myTarget_generateMacros()
        
        - FILE* inFp - file created by init() but onlywith read permission.
            (position is set AFTER '$m" but BEFORE '\n' character)
       
        - FILE* outFp - output file .h - function should write macros to this file
        
        -  const TARGET_FLAGS* fls - as in init()
        
        -  returned value:
            -  _-1_ in case of error (function should print info about error)
            
            -  or nonnegative number of pins for whose function generated macros
        
        - this function should read data from input file and convert it into macros in output file.
            Macros should use IDENTICALL syntax as macros for other targets. If it's impossible, please contact me. 
        
        - 

    - myTarget_help()
        
        - some info about _target_

- you can create other functions if needed, but their declarations should be placed __only__ in your .c file

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

