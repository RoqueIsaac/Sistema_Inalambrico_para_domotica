//#include <xc.h> // include processor files - each processor file is guarded.  

void internal_clock()
{
    //Osc internal 8 mhz
    OSCCONbits.IRCF0 = 1;  //clock 8Mhz   111=8;   110=4;    101=2; 100=1;
    OSCCONbits.IRCF1 = 1;  //clock 8Mhz   011=0.5; 010=0.25; 001=0.125
    OSCCONbits.IRCF2 = 1;  //clock 8Mhz
    OSCCONbits.SCS1 = 1;   //internal Osc
}


void config_reg(){
    //Puertos Analogicos como Digitales
    //PORT A,B,E,
    ADCON1=0x0F;
    //puerto A0 entrada de sensores: PIR
    TRISA = 0x01;
    //as output
    TRISB = 0x00;
    //led sensores
    TRISCbits.RC0 = 0;
    //input pushbuttons para cambio de edo. manual
    TRISD = 0xFF;
    //posiblemente entrada de lector RFD wiegand, RE tiene solo 3 bits.
    //TRISE = 0;
    
    //Conf Timer
    //TMRO = 65535 - (T*Fos/(4*Preescaler))
    T0CONbits.T08BIT = 0;   //Timer de 16 bits
    T0CONbits.PSA = 0;      //preescaler activado
    T0CONbits.T0CS = 0;     //reloj interno para timer
    T0CONbits.T0PS = 7;     //preescaler para 256
    // 5 Seg para 8 Mhz
    //TMR0 = 65535 - ((5seg * 8Mhz)/(4*256))
    TMR0 = 26472;           //TMR0 para timer de 5 seg
    T0CONbits.TMR0ON = 0;    //deshabilita el timer
    
    //cuando se escribe en EEPROM, se tiene que desahabilitar intr.
    // de otra manera, el pic se cuelga, porque la rutina de escritura
    // se interrumpe el PC se pierde, por lo que la secuencia de programa
    //se pierde.
    INTCONbits.TMR0IE = 0;  //habiitar interrupcion de timer
    INTCONbits.TMR0IF = 0;  //limpiar bandera de interrupcion de timer   
       
}


// Interrupciones
void RX_interrupt_enabled(){
    //Enable global Interrups
    INTCONbits.GIE = 1;     //enable global interrupt
    INTCONbits.PEIE = 1;    //enable peripheral interrupt
    RCONbits.IPEN = 0;      //prioridad de interrupcion off
    
    //flags
    PIR1bits.RCIF = 0;     //RX interrupt flag
    //PIR1bits.TXIF = 0;   //TX interrupt flag        

    PIE1bits.RCIE = 1;      //Enable RX interrupt
    RCSTAbits.CREN = 1;     //Continuos RX
    //IPR1bits.RCIP = 1;    //high priority RX interrupt
    
    //PIE1bits.TXIE = 1;    //Enable TX interrupt
    //IPR1bits.TXIP = 0;    //high priority TX interrups
}


/*void RX_interrupt_disabled(){
    //Enable global Interrups
    PIE1bits.RCIE =  0;     //disaable RX interrupt
    RCSTAbits.CREN = 0;     //Continuos RX, no RX
    INTCONbits.GIE = 0;    //enable global interrupt
    INTCONbits.PEIE =0;    //enable peripheral interrupt
    //RCONbits.IPEN =  0;     //prioridad de interrupcion off
    
  
    //flags
    PIR1bits.RCIF = 0;     //RX interrupt flag
    //PIR1bits.TXIF = 0;    //TX interrupt flag        
  
    //IPR1bits.RCIP = 1;   //high priority RX interrupt

    //PIE1bits.TXIE = 0;   //Enable TX interrupt
    //IPR1bits.TXIP = 0;   //high priority TX interrups
    
}
 
*/



