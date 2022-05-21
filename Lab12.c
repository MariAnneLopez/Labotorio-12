/* 
 * File:   Lab12.c
 * Author: Marian López
 *
 * Created on 18 de mayo de 2022, 07:30 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 4000000

// VARIABLES
int bandera = 2;
int pot = 0;

// PROTOTIPOS DE FUNCIONES
void setup(void); 
int read_EEPROM(int address);
int write_EEPROM(int address, int data);

// INTERRUPCION
void __interrupt() isr (void){
    if(PIR1bits.ADIF){                  // Fue int. ADC?
        if(ADCON0bits.CHS == 0){        // Fuer int. AN0?
            pot = ADRESH;            
            PORTC = pot;     
        }
        PIR1bits.ADIF = 0;
    }
    if(INTCONbits.RBIF){                // Fue int. PORTB?
        if(!PORTBbits.RB0){             // Fue RB0?
            bandera = 1;    
        }              
        if(!PORTBbits.RB1){             // Fue RB1?
            bandera = 0;
            PORTD = read_EEPROM(1);     // Leer EEPROM
        }
        if(!PORTBbits.RB2){             // Fue RB2?
            write_EEPROM(1, pot);
            bandera = 0;
            PORTD = read_EEPROM(1);     // Leer EEPROM
        }
        if(!PORTBbits.RB3){
            PORTD = read_EEPROM(1);
        }
        INTCONbits.RBIF = 0;
    } 
    return;
}

// CICLO PRINCIPAL
void main(void) {
    setup();
    while(1){ 
        if (bandera == 1){
            SLEEP();
        }
        if(ADCON0bits.GO == 0){         // Si no hay proceso de conversión
            ADCON0bits.GO = 1;          // Inicia la conversión
        }
    }
    return;
}   

// CONFIGURACIONES
void setup(void){
    ANSEL = 1;          // AN0  analogico
    ANSELH = 0;
    
    TRISA = 1;          // AN0 entrada
    TRISC = 0;
    TRISD = 0;
    
    PORTA = 0; 
    PORTC = 0;
    PORTD = 0;
    
    OPTION_REGbits.nRBPU = 0;   // Pull ups encendidas individualmente
    WPUB = 0b1111;                // Pull ups encendidas
    IOCB = 0b1111;                // Int. por cambio de estado encendidas
    
    // Configuracion reloj interno
    OSCCONbits.IRCF = 0b0110;   // 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuracion ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON1bits.VCFG0 = 0;       // VDD *Referencias internas
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0;         // Canal AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitar modulo ADC
    __delay_us(40);
    
    // Interrupciones
    PIR1bits.ADIF = 0;          // Limpiar bandera de int. ADC
    PIE1bits.ADIE = 1;          // Habilitar int. ADC 
    INTCONbits.RBIF = 0;        // Limpiar bandera de int. PORTB
    INTCONbits.RBIE = 1;        // Habilitar int de PORTB
    INTCONbits.PEIE = 1;        // Habilitar int. de periféricos
    INTCONbits.GIE = 1;         // Habilitar int. globales 
}

int read_EEPROM(int address){
    EEADR = address;        // Escribir la dirección
    EECON1bits.EEPGD = 0;   // Acceder a la EEPROM
    EECON1bits.RD = 1;      // Inicia la lectura
    return EEDAT;           // Regresa dato leido
}

int write_EEPROM(int address, int data){
    EEADR = address;
    EEDAT = data;
    EECON1bits.EEPGD = 0;   // Escribir la EEPROM
    EECON1bits.WREN = 1;    // Habilitar escritura en la EEPROM
    
    INTCONbits.GIE = 0;     // Apagar int. globales para no interrumpir escritura
    EECON2 = 0x55;          // Preparar memoria para recibir datos
    EECON2 = 0xAA;  
    
    EECON1bits.WR = 1;      // Iniciar escritura
    EECON1bits.WREN = 0;    // Deshabilitar para que no pase nada del uC a la EEPROM
    INTCONbits.RBIF = 0;
    INTCONbits.GIE = 1;
}
