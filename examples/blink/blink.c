#include <avr/io.h>
#include <util/delay.h>

#include "gpiodefs.h"

/*
How m-gen can be used - minimal working example.

*/


int main(void)
{
    LED1_asOutput();
    
    while(1)
    {
        LED1_On();
        _delay_ms(1000);
        
        LED1_Off();
        _delay_ms(500);
    }
}
