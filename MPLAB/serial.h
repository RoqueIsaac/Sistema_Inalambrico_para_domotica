/* 
 * File:   serial.h
 * Author: Roque Isaac Gomez Z.
 * Comments: Serial Asincrono library for pic 18F4550
 * Revision history: 
 */
//#include <stdio.h>
//#include <stdlib.h>

void serial_open(unsigned int baud){
    //RCSTA register as default is 0x00
    //TXSTA register as default is 0x02 --> TSR empty
    //BAUSCON as defailt is 0b0100 0x00
    //-----------------------
    //Librreria calculada para 8 Mhz Osc y 9600 bauds 
    /*Calculando valor de SPBRG (variable x) para 8Mhz
    x = ((Foc / BR*64)-1);
    x = (8000000/(9600*64))-1;
    x = 12.02  -->  x = SPBRG = 12; 
    BRcalculated = Fosc / (64*(n+1));
    BRcalculated = 9615.38        ;
    Error = (BRcal - 9600) / 9600;   
    error = 0.16 %; 
*/
    //Conf para 9600 Bauds
    int bds;
    TXSTAbits.SYNC = 0;     //Modo Asyncrono
    BAUDCONbits.BRG16 = 0;  //16 bits Baud Rate generator, 0 = only 8 bits
    TXSTAbits.BRGH = 1;             //usando high speed
    SPBRGH = 0;         //usando solo 8 bits para el baud rate generator, parte alta en 0, registro de 2 bytes..
    SPBRG = (unsigned char)(((_XTAL_FREQ/baud)/16)-1); //calculo de sprg para generar el baud_rate, de acuerdo al cristal
 
/*    
    bds = (((_XTAL_FREQ/baud)/16)-1); //calculo de sprg para generar el baud_rate, de acuerdo al cristal
    if (bds>255){
        TXSTAbits.BRGH = 0;             //usando high speed
        BAUDCONbits.BRG16 = 1;  //16 bits Baud Rate generator, 0 = only 8 bits
        SPBRG=255;
        SPBRGH=(unsigned char)(bds-255);
    }else{
        TXSTAbits.BRGH = 1;             //usando high speed
        BAUDCONbits.BRG16 = 0;  //16 bits Baud Rate generator, 0 = only 8 bits
        SPBRG=(unsigned char)bds;
        SPBRGH=0;
    }            
*/    
    //-----------------------
    //TXSTAbits.CSRC = x;      // dont care Asynchronous, Clock source select bit
    TXSTAbits.TX9 = 0;        //8 bits TX
    //TXSTAbits.SENDB = 0     //Send break character bit
    //TXSTAbits.TX9D = 0      //only for 9 bits serail TX
    //----------------------    
    RCSTAbits.RX9 = 0;        //8 bits RX
    //RCSTAbits.SREN = 0      //single RX  enable bit, only for synchronous
    RCSTAbits.CREN = 1;       //Continuos RX
    //RCSTAbits.ADDEN = 0;    //mod 9 bits, disable address detection
    //RCSTAbits.FERR = 0;     //Framing Error bit
    //RCSTAbits.OERR = 0;     //Overrun error bit
    //RCSTAbits.RX9D = 0;     //9th bit of data RX
    //----------------------
    //BAUDCONbits.ABDOVF = 0; //Auto baud acquisition rollover
    //BAUDCONbits.RCIDL = 0;  //Recieve Op idle status bit
    BAUDCONbits.RXDTP = 0;    //Rx data polarity 0 = no inverted
    BAUDCONbits.TXCKP = 0;    //Clock data polarity 0 = no inverted
    //BAUDCONbits.WUE = 1;    //Wake up enable bit, usart sample rx, interrupt gen on falling edge
    //BAUDCONbits.ABDEN = 0;  //Baud rate measurement
    //----------------------

    TRISCbits.RC7 = 1;      //Pin RX como entrada
    TRISCbits.RC6 = 1;      //Pin TX como salida, aunque salida debe ser 1, el pic los cambia a conveniencia
    
    RCSTAbits.SPEN = 1;     //Habilita Serial port
    TXSTAbits.TXEN = 1;     // TX enable
    
    //----------------------
}


void serial_close(void){
    RCSTAbits.SPEN = 0;     //Habilita Serial port
}


char TX_busy(void){
    return (char) !TXSTAbits.TRMT;    //0 TSR full; 1 TSR empty
}

void serial_write(char word[]){
    int i;
    for (i=0; word[i]!=(0x00); i++){  //search for end string character on string
        TXREG = word[i];            //load 8 bits on TXREG register to transmit, serial RX begin.
        while(TX_busy());   //wait until TSR (transmit shift register get empty) sent the byte. 0=TSR full; 1=TSR empty
    }        
}


void clear_buffer(){
    int i;
    char y;
    for (i=0;i<5;i++){
        y=RCREG;
    }
}
//Lee 1 caracter del serial

/*
unsigned char single_char(){
    //unsigned char value[];
    char x;
    if (PIR1bits.RCIF==1){;  //wait for reception, RCIF, will be set when reception is complete & interrupt will be generated if RCIE, was set.
           return RCREG;
    }
    else{
        //clear_buffer();
        return '\0';
    }
}
*/

//Revisa si hay datos en el serial.
char RX_ready(void){
     return (char)PIR1bits.RCIF;
}

//Lee una cadena de caracteres, hasta un que encuentra el 
//caracter conocido ?, que es fin de cadena.
void serial_read(char info[]) {
    int i = 0;      
    while (1) {
        while (!RX_ready());
        info[i] = RCREG;
        //PIR1bits.RC1IF=0;
        if (info[i] != '_') {
            i++;
        }
        else {
            //PIR1bits.RC1IF=0;
            //RCSTAbits.CREN=0;    //se limpia, en caso de existir el over-run
            //RCSTAbits.CREN=1;
            break;
        }
        if (i>5){
            info = 0x00; 
            //RCSTAbits.CREN = 0;
            //RCSTAbits.CREN = 1;              
            break;
        }
    }       
}




/*
unsigned char serial_read(){
    unsigned char word[];
    unsigned int j = 0;
    while(1){
        if (PIR1bits.RCIF==1){;  //wait for reception, RCIF, will be set when reception is complete & interrupt will be generated if RCIE, was set.
           word[j] = RCREG;
           //PIR1bits.RCIF = 0;
           if (word[j] != '\0'){
               j++;             //if it is not a carriage return (enter)
           }
           else {
               //word[j+1]='\0';     //Add end of string
               return word;
           }
        }
        else
            return;
    }        //en la rutina de interrupcion debo preguntar si el caracter es el fin del string
}           // de otro modo no se voy a poder seguir leyendo cuando entre la interrupcion.
*/









