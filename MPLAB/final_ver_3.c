/*
 * File:   prefinal_5.c
 *
 * 28 de Septiembre de 2019
 * Contiene lectura de sensor PIR en RA0, led en RC0
 * Guadra ultimo status en EEPROM
 * Se agregó 1 byte al envío para revisar inactividad
 * Se agrego interrupcion por Recepcion RX
 * Se agregó lectura de PORTD, para act manual de Serv
 */

#define _XTAL_FREQ 8000000 //Freq final del micro depues de postscaler
#include <xc.h>
#include <pic18f4550.h>
//para osc interno de 8 Mhz
#include "FusesCom.h"
#include "serial.h"
#include "config_reg.h"
#include "EEMPROM_Libreria.h"

#define AULA 8
#define EDIF 'A'
#define LED_PIR LATCbits.LC0  //LED de inactividad debido a PIR en RC0

//en un primer inicio, todos los servicios activos
__EEPROM_DATA(0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
 
//global var
char backup;
char inpt[10];
char x[6]={0x00,0x00,0x00,0x00,0x00,0x00};
unsigned int contador=0; //var para contar segundos mult de 5

void interrupt RXx(void);
void Delay1Second();

void main() {
    
    serial_close();  
    internal_clock(); 
    config_reg();    
    //Despliega en PORTB valor del primer byte de EEPROM, ultimo valor conocido
    LATB = EEPROM_Lectura(0);
    LATCbits.LATC0=0;
    backup = EEPROM_Lectura(0);
  
    //Habilita interrupcion RX
    RX_interrupt_enabled();
    
    clear_buffer();
    serial_open(9600);
    
    TMR0 = 26472;           //TMR0 para timer de 5 seg
    T0CONbits.TMR0ON = 1;   //habilita el timer    
    while(1) {     
    //Espera interrupcion por RX       
    /////////// SERVICIOS MANUAL ///////////
        if ((PORTDbits.RD0==1)||(PORTDbits.RD1==1)||(PORTDbits.RD2==1)||(PORTDbits.RD3==1)||(PORTDbits.RD4==1)){                   
            if (PORTDbits.RD0==1) LATBbits.LATB0=!PORTBbits.RB0;
            if (PORTDbits.RD1==1) LATBbits.LATB1=!PORTBbits.RB1;
            if (PORTDbits.RD2==1) LATBbits.LATB2=!PORTBbits.RB2;
            if (PORTDbits.RD3==1) LATBbits.LATB3=!PORTBbits.RB3;
            if (PORTDbits.RD4==1) LATBbits.LATB4=!PORTBbits.RB4;
            //if (PORTDbits.RD5==1) LATBbits.LATB5=!PORTBbits.RB5;
            //if (PORTDbits.RD6==1) LATBbits.LATB6=!PORTBbits.RB6;
            //if (PORTDbits.RD7==1) LATBbits.LATB7=!PORTBbits.RB7;            
            LED_PIR=0;
            contador = 0;
            backup=PORTB;
            TMR0 = 26472;           //TMR0 para timer de 5 seg
            INTCONbits.TMR0IF = 0;
            Delay1Second();
        }       
    /////////// PARA TIMER + PIR ///////////
        if (PORTAbits.RA0==0){
            if (INTCONbits.TMR0IF == 1){
                contador ++; 
                if (contador == 2){     //timer es cada 5 seg, x tanto es = 10 seg
                    LED_PIR=1;
                    backup=PORTB;
                    LATB = 0x00;
                }
                //if (contador > 2) contador = 3; //solo para evitar desbordamiento
                TMR0 = 26472;           //TMR0 para timer de 5 seg
                INTCONbits.TMR0IF = 0;
            }
        }
        if (PORTAbits.RA0==1){ //hay movimiento
            LED_PIR=0;
            contador = 0;
            TMR0 = 26472;           //TMR0 para timer de 5 seg
            LATB=backup;
        }
    ////////////////////////////////////////
        
    } 
}

void interrupt RXx(void){
    serial_read(inpt); //si no se pide status el programa se quede en esta funcion, hasta que lee algo en el serial
    if (inpt[0]==EDIF && inpt[1]==(AULA+33) && inpt[3]=='_'){
        //RCSTAbits.CREN = 0;       //Desactivar RX
        if (inpt[2]=='s'){ //Solicita Status de monitor   
            x[0]=EDIF;              //Edificio
            x[1]=(unsigned char)(AULA+33);     //Se agregan aula+33 para evitar el ascii 10 que es salto de linea
            x[2]=PORTB;
            x[2]=(unsigned char)(x[2]+33);     //Se agregan 33 para evitar el ascii 10 que es salto de linea
            x[3]=(unsigned char)(PORTCbits.RC0+33); //bit de inactividad viene del PIR
            x[4]='?';
            serial_write(x);
        }
        else if (inpt[2]=='t'){ //Solicita Status de sub_vi
            x[0]=EDIF;              //Edificio
            x[1]=(unsigned char)(AULA+33);     //Se agregan aula+33 para evitar el ascii 10 que es salto de linea
            x[2]=backup;
            x[2]=(unsigned char)(x[2]+33);     //Se agregan 33 para evitar el ascii 10 que es salto de linea
            x[3]=(unsigned char)(PORTCbits.RC0+33); //bit de inactividad viene del PIR
            x[4]='?';
            serial_write(x);
        }
        else{ //escribir dato en PORTB (nuevo dato de servicios)
            inpt[2]=(char)(inpt[2]-33);
            LATB=inpt[2];
            backup = inpt[2];                                               
            EEPROM_Guardar(0,inpt[2]);
            LED_PIR=0;    //apaga led de inactividad
            contador = 0; //re-establece conteo
            TMR0 = 26472;           //TMR0 para timer de 5 seg
            INTCONbits.TMR0IF = 0;  //Clear timer flag                                  
        }   
    }
    PIR1bits.RCIF = 0;     //RX interrupt flag;
}



void Delay1Second(){
    int i = 0;
    for(i=0;i<50;i++){
         __delay_ms(10);
    }
}
 

