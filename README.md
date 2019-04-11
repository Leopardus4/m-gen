# m-gen

> Macro - GENerator


## What is it

_m-gen_ is a simple program which can creates easy-to-use, fast-to-make, and easy-to-port C/C++ code (drivers layer) for accessing gpio in microcontrollers.

## How it works

- fast
- ...

### Usage:

(In shell)

        cd /path/to/project/

- First call creates empty sheet of macros prototypes

        m-gen my_new_file.gm --init -t my_microcontroller

- Open this file. You should see sth like this:

        $t
        mcu_type
        
        $c
        Here place your comment about file
        
        $m
        (This is main part of file. Here you should write your prototypes of macros - at the end of file you should see full instruction. Example :
        Mode PORT PIN Name Comment
        o   B   2   HC59X_data1 Serial data input (8 bit mode)
        o   B   3   HC59X_clk   Serial clock input
        b   D   5   userButton1   Button on PCB


    You can write as many prototypes as you want.


- Then call _m-gen_ without _--init_ parameter:


        m-gen my_file.gm -o other_name.h

    or

        m-gen my_file.gm

    (it will creates my_file.h)

- New .h file contains auto-generated macros like this:

        //------------------------------------------------------------------------//
        
        /* HC59X_data1 - PB2 - digital output 
        	  Serial data input (8 bit mode)
        */
        
        #define HC59X_data1_dirOut()     do{DDRB |= (1<<PB2);} while(0)
        
        #define HC59X_data1_setHigh()    do{PORTB |= (1<<PB2);} while(0)
        
        #define HC59X_data1_setLow()     do{PORTB &= ~(1<<PB2);} while(0)
        
        
        //------------------------------------------------------------------------//


    You can use these macros in your project.


- If you must change i. e. PD5 pin to PC2, just edit .gm file and run m-gen.
    It overrides your .h file - macros have pin-independent names - you don't have to edit either one line of code.
    It's also possible to change mcu family to another - just run _m-gen --init_ with other _-t_ parameter and other filename,
    copy prototypes from previous file and replace only chip-dependent elements (port, pin).


- For more informations, see _m-gen --help_


## Installation

1. Precompiled binaries:
    - download executables from [RELEASE page](https://github.com/Leopardus4/m-gen/releases) for your system

    - just copy m-gen to directory in PATH:

        * Linux and macOS: /usr/local/bin/

        * Windows: C:\new_dir\ (you must add this directory to [PATH](https://www.howtogeek.com/118594/how-to-edit-your-system-path-for-easy-command-line-access/) )

2. From sources:

    - download sources
    
    - (on Windows you need to install "make.exe" before compiling)
    
    - (set your compiler in Makefile - default value: _COMPILER = cc_ )

    - run (in m-gen directory):

            make all
    
    - next:

        - on Linux / macOS :

                sudo make install

        - Windows: Copy file bin/m-gen to i. e. C:\new_folder and add it to PATH




### Test

You can test if it works - open shell in random directory and type

    m-gen --version


---

## License

This program is distributed under _GPLv3_ license (see: LICENSE.txt)


## Supported MCU-s

- Atmel AVR family (ATtiny & ATmega)

- NXP LPC 111x (ARM Cortex M0 based)

- 17xx (not yet)

Cannot find your MCU? Nothing wrong, it's easy to add it. See CONTRIBUTING.md for details.



