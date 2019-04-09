#include <avr/io.h>
#include <util/delay.h>

#include "gpiodefs.h"

/*
How m-gen can be used - minimal working example.

*/


void init(void)
{
    LED1_asOutput();
    MainButton_asInput();
}


int main(void)
{
    init();
    
    
    while(1)
    {
        LED1_On();
        
        
        if(MainButton_isActive())
            _delay_ms(250);
        
        else
            _delay_ms(1000);
        
        
        
        LED1_Off();
        
        
        if(MainButton_isActive())
            _delay_ms(250);
        
        else
            _delay_ms(500);
    }
}
