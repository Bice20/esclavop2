/* 
 * File:   P2P2esclavo.c
 * Author: Aida Toloza, Brandon Cruz
 *
 * Created on 30 de mayo de 2022, 11:23 AM
 */

#pragma config FOSC = INTRC_NOCLKOUT
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config MCLRE = OFF
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF
#pragma config LVP = OFF

// CONFIG2
#pragma config BOR4V = BOR40V
#pragma config WRT = OFF

// librerias

#include <xc.h>
#include <stdint.h>

// constantes
#define _XTAL_FREQ  4000000             // Constante para __delay_us

//varibles
uint8_t data_SPI = 0;

//Funciones

void setup(void);                      // Configuración del PIC
void reset_TMR0(void);                 // Resetear TMR0

//Interrupciones
void __interrupt() isr (void)           
{
    // para comenzar realizamos la configuración para realizar la Interrupcion ADC
    if (PIR1bits.ADIF)                  
    {   
        PIR1bits.ADIF = 0;              // esto lo que hace es desactiva la interrupción
    }
    
    // posteriomente seguimos con la configuración para realizar la Interrupción del  TMR0
    if (INTCONbits.T0IF)                
    {
        reset_TMR0();                   // Crealizamos la configuración para poder Resetear el TMR0
        INTCONbits.T0IF = 0;            // posteriormente la configuración de desactivar las interrupcion
    }
    
    // también realizamos la configuración para realizar la Interrupcion SPI
    if (PIR1bits.SSPIF)
    {
        PIR1bits.SSPIF = 0;             // lo que se realiza es la configuración para desactivar interrupcion
    }
}

//Main

void main(void)                         
{
    setup();                            
    reset_TMR0();                       
    __delay_us(50);
    
    while (1)                         
    {
        // lo que se tiene que realziar es el control de canales con el propocito de 
        // iniciar la conversion ADC
       
        if (ADCON0bits.GO == 0)         
        {
            if (ADCON0bits.CHS == 0b0000)           // cambio de Canal 0000 -> 0001
            {
                ADCON0bits.CHS = 0b0001;
            }
            
            else if (ADCON0bits.CHS == 0b0001)      //cambio de  Canal 0001 -> 0010
            {
                ADCON0bits.CHS = 0b0010;
            }
            
            else if (ADCON0bits.CHS == 0b0010)      // cambio de  Canal 0010 -> 0011
            {
                ADCON0bits.CHS = 0b0011;
            }
            
            else if (ADCON0bits.CHS == 0b0011)       // cambio de Canal 0011 -> 0000
            {
                ADCON0bits.CHS = 0b0000;
            }
            
            __delay_us(50);             
            ADCON0bits.GO = 1;          
        }
        
        // lo que es muy importante para que tood funcione es la 
        // configuración para realizar la comunicacion Maestro-Esclavo (SPI)
        
        while(!SSPSTATbits.BF);         // utilizamos lacondición while la cual nos va a ayudar a pode realizar la esperar de la lectura de datos
        data_SPI = SSPBUF;              // posteriormente a esto se va a realizar el almacenamiento de los datos que se recibieron
        
         // Posteriormente realizamos la configuración de Servo 1 el cual va a guardar, encender o apagar dependiendo de
        // que tipos de indicaciones le brindemos
        if (PORTBbits.RB0)
        {
            CCPR1L = (data_SPI>>1)+123;
            CCP1CONbits.DC1B = (data_SPI & 0b01);
            CCP1CONbits.DC1B0 = (data_SPI>>7);
        }
        
        // Posteriormente realizamos la configuración de Servo 2 el cual va a guardar, encender o apagar dependiendo de
        // que tipos de indicaciones le brindemos
        else if (PORTBbits.RB1)
        {
            CCPR2L = (data_SPI>>1)+123;
            CCP1CONbits.DC1B = (data_SPI & 0b01);
            CCP1CONbits.DC1B0 = (data_SPI>>7);
        }
    }
    
    return;
}
// después de realizar la configuración de los dos primeros servos ahora comenzamos con la configuración de el pic

void setup(void)                        
{
    // I/O
    ANSEL = 0x0F;                       //Definimos las Entradas analógicas
    ANSELH = 0;                         //a su misma vez las  I/O digitales
    
    // Oscilador
    OSCCONbits.IRCF = 0b0111;           // especificamos que el oscilador interno es de 4MHz
    OSCCONbits.SCS = 1;                 // Oscilador interno
    
    // Definimos cuales van a ser las entradas
    TRISA = 0x23;                       // puerto A
    PORTA = 0;                          
    
    TRISB = 0x03;                       // Puerto B                 
    PORTB = 0;     
    
    TRISC = 0x18;                       // Puerto C
    PORTC = 0;
    
    TRISD = 0x00;                          
    PORTD = 0;       
    
    TRISE = 0x00;
    PORTE = 0;
    
    // ADC & PWMs
    ADCON0bits.ADCS = 0b01;             // Reloj de conversión: Fosc/8
    ADCON0bits.CHS = 0b0000;            // Canal para pin AN0
    ADCON1bits.VCFG0 = 0;               // Ref: VDD
    ADCON1bits.VCFG1 = 0;               // Ref: VSS
    ADCON1bits.ADFM = 0;                // Lo Justificado a la izquierda
    ADCON0bits.ADON = 1;                // Habilitar ADC

    TRISCbits.TRISC2 = 1;               // Deshabilitar salida en CCP1
    CCP1CONbits.P1M = 0;                // Modo Single Output
    CCP1CONbits.CCP1M = 0b1100;         // PWM 1
    CCP2CONbits.CCP2M = 0b1111;         // PWM 2 
    
    CCPR1L = 0x0F;                    
    CCP1CONbits.DC1B = 0;     
    CCPR2L = 0x0F;
    PR2 = 250;               
    
    T2CONbits.T2CKPS = 0b11;
    T2CONbits.TMR2ON = 1;
    PIR1bits.TMR2IF = 0;

    while(PIR1bits.TMR2IF == 0);
        PIR1bits.TMR2IF = 0;
        TRISCbits.TRISC1 = 0;
        TRISCbits.TRISC2 = 0;
    
    TRISCbits.TRISC2 = 0;               // realizamos la salida de PWM
    
    // TMR0
    OPTION_REGbits.T0CS = 0;            // Reloj interno para el TMR0
    OPTION_REGbits.T0SE = 0;            // Flanco de reloj ascendente
    OPTION_REGbits.PS2 = 1;             // 
    OPTION_REGbits.PS1 = 1;             //    -> Prescaler: 1:256
    OPTION_REGbits.PS0 = 1;             //
    
    // SPI
    SSPCONbits.SSPM = 0b0100;           // SPI: Slave mode, Clock: Fosc/4
    SSPCONbits.CKP = 0;                 // desactivamos el reloj al inicio
    SSPCONbits.SSPEN = 1;               // se realiza la habilitación de los pines SPI
    SSPSTATbits.CKE = 1;                // Aqui realizamos la configuramos para que lo pueda transmitir en el  flanco positivo 
    SSPSTATbits.SMP = 1;                // y finalmente este tiene que Enviar al final del pulso de reloj
    
    // Interrupciones
    INTCONbits.GIE = 1;                 // configuración de interrupciones globales
    INTCONbits.PEIE = 1;                // configuración de interrupciones Perifericas
    
    PIE1bits.ADIE = 1;                  // ADC
    PIR1bits.ADIF = 0;                  // Bandera ADC
    
    INTCONbits.T0IF = 0;                // TMR0
    INTCONbits.T0IE = 1;                // Bandera TMR0
    
    PIE1bits.SSPIE = 1;                 // SPI
    PIR1bits.SSPIF = 0;                 // Bandera SPI
    
    //Configuración de valores iniciales
    PORTBbits.RB0 = 1;                  // Indicador esclavo
}

void reset_TMR0(void)                   // Resetear TMR0
{
    TMR0 = 0;                           // Prescaler
    INTCONbits.T0IF = 0;                // Limpiar bandera de interrupcion
    return;
}