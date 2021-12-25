#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config CP = OFF         // FLASH Program Memory Code Protection bits (Code protection off)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low Voltage In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection (Code Protection off)
#pragma config WRT = ON         // FLASH Program Memory Write Enable (Unprotected program memory may be written to by EECON control)


#include <xc.h>

unsigned char flag = 1;


void main(void) {
    
    unsigned int i, j, k;
    
    
    unsigned char LEDS[8][5]={
        {0x81,0x42,0x24,0x18,0x24,0x42,0x81,0xFF},
        {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
        {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01},
        {0xAA,0x00,0xAA,0x00,0xAA,0x00,0xAA,0x00},
        {0x55,0x00,0x55,0x00,0x55,0x00,0x55,0x00,}
    };
   
    TRISB = 0x01; //Definir botao como entrada
    PORTC = 0x00; 
    TRISC = 0x00; //Definir C como saida
    
    INTCONbits.INTF=0;
    INTCONbits.INTE=1; //RB0
    INTCONbits.GIE=1;
    

    while(1){
        
        for(j=0; j<8; j++){
            
            PORTC = LEDS[j][flag];
        }
         
        for(i=1; i<60000; i++);
                            
    }
    
    return;
}


void __interrupt() ISR(void){
    
    if(INTCONbits.INTF){
        flag ^= 1;
        
        INTCONbits.INTF=0;
    }
   
    
}
