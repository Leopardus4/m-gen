# Example of m-gen in Makefile


#compiler, etc.
PREFIX = avr-

CC 		= ${PREFIX}gcc
OBJCOPY = ${PREFIX}objcopy


M_GEN = m-gen
# M_GEN = m-gen.exe	  # Windows




# main file name
MAIN = blink.c


# m-gen header name
DEF_HEADER = gpiodefs.h

# m-gen array (.gm)
DEF_ARRAY = gpiodefs.gm



# AVR chip
CHIP = atmega8

# F_CPU frequency
F_CPU = 1000000UL


#optimalization level
OPT = -Os

# compiler flags
CFLAGS = -Wall		\
		-std=c99	\
		${OPT}		\
		-mmcu=${CHIP}	\
		-DF_CPU=${F_CPU}


# linker flags
LFLAGS = ${CFLAGS}	\
		-lm




################################

# .o files
OBJS	= ${MAIN:%.c=%.o}


# output hex file
PROG	= ${MAIN:%.c=%.hex}




################################
# rules

all: ${PROG}



%.hex : %.elf
	${OBJCOPY} -O ihex -R .eeprom $< $@



%.elf : ${OBJS}
	${CC} ${LFLAGS} $^ -o $@



%.o : %.c ${DEF_HEADER}
	${CC} -c ${CFLAGS} $< -o $@




# How to use m-gen in Makefile:

${DEF_HEADER} : ${DEF_ARRAY}
	${M_GEN} -s $< -o $@




###

clean : 
	@ echo cleaning...
	rm -f *.hex *.elf *.o


.PRECIOUS : %.o %.elf
.PHONY : all clean

