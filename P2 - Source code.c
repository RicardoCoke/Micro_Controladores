/*
 * File:   newmain.c
 * Author: Pedro Xavier Magalhães, 60302 & Ricardo Coke, 61368
 *
 * Created on January 22, 2021, 17:45
 * 
 * Description: Programa para PIC16F876 envolvendo ADC e comunicação I2C.
 * O ADC do PIC lê dados das portas analógicas A0, A1, e A2. Cada porta recebe um input entre 0 e 5 volts.
 * O PIC calcula a média aritmética das tensões lidas nas três portas e envia o resultado para um display usando o protocolo I2C.
 */

// PIC16F876 Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config CP = OFF         // FLASH Program Memory Code Protection bits (Code protection off)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low Voltage In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection (Code Protection off)
#pragma config WRT = ON         // FLASH Program Memory Write Enable (Unprotected program memory may be written to by EECON control)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#define _XTAL_FREQ 2000000

#define SSD1306_I2C_ADDRESS          0x7A
#define SSD1306_LCDWIDTH             128
#define SSD1306_LCDHEIGHT            64
#define SSD1306_SETCONTRAST          0x81
#define SSD1306_DISPLAYALLON_RESUME  0xA4
#define SSD1306_DISPLAYALLON         0xA5
#define SSD1306_NORMALDISPLAY        0xA6
#define SSD1306_INVERTDISPLAY_       0xA7
#define SSD1306_DISPLAYOFF           0xAE
#define SSD1306_DISPLAYON            0xAF
#define SSD1306_SETDISPLAYOFFSET     0xD3
#define SSD1306_SETCOMPINS           0xDA
#define SSD1306_SETVCOMDETECT        0xDB
#define SSD1306_SETDISPLAYCLOCKDIV   0xD5
#define SSD1306_SETPRECHARGE         0xD9
#define SSD1306_SETMULTIPLEX         0xA8
#define SSD1306_SETLOWCOLUMN         0x00
#define SSD1306_SETHIGHCOLUMN        0x10
#define SSD1306_SETSTARTLINE         0x40
#define SSD1306_MEMORYMODE           0x20
#define SSD1306_COLUMNADDR           0x21
#define SSD1306_PAGEADDR             0x22
#define SSD1306_COMSCANINC           0xC0
#define SSD1306_COMSCANDEC           0xC8
#define SSD1306_SEGREMAP             0xA0
#define SSD1306_CHARGEPUMP           0x8D
#define SSD1306_EXTERNALVCC          0x01
#define SSD1306_SWITCHCAPVCC         0x02


// Referência de código fonte:
// http://www.picguides.com/beginner/adc.php

// PIC16F876 possui ADC de 10 bits.

void initADC(void) {
    ADCON0 = 0xC1;
    ADCON1 = 0x80;
}

unsigned short readADC(unsigned char channel) {
    if (channel > 7) return 0;

    ADCON0bits.CHS = channel; // Definir bits de seleção de canal analógico a ler.
    __delay_ms(5); // Esperar tempo de aquisição.
    ADCON0bits.GO_nDONE = 1; // Iniciar conversão colocando "A/D conversion status bit" a 1.
    while (ADCON0bits.GO_nDONE); // Aguardar pelo fim da conversão.
    return ((ADRESH << 8) + ADRESL); // Retornar resultado.
}


uint8_t _i2caddr, _vccstate, x_pos = 1, y_pos = 1;

const char Font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x5F, 0x00, 0x00,
    0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x56, 0x20, 0x50,
    0x00, 0x08, 0x07, 0x03, 0x00,
    0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00,
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
    0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x80, 0x70, 0x30, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x00, 0x60, 0x60, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02,
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    0x00, 0x42, 0x7F, 0x40, 0x00,
    0x72, 0x49, 0x49, 0x49, 0x46,
    0x21, 0x41, 0x49, 0x4D, 0x33,
    0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39,
    0x3C, 0x4A, 0x49, 0x49, 0x31,
    0x41, 0x21, 0x11, 0x09, 0x07,
    0x36, 0x49, 0x49, 0x49, 0x36,
    0x46, 0x49, 0x49, 0x29, 0x1E,
    0x00, 0x00, 0x14, 0x00, 0x00,
    0x00, 0x40, 0x34, 0x00, 0x00,
    0x00, 0x08, 0x14, 0x22, 0x41,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08,
    0x02, 0x01, 0x59, 0x09, 0x06,
    0x3E, 0x41, 0x5D, 0x59, 0x4E,
    0x7C, 0x12, 0x11, 0x12, 0x7C,
    0x7F, 0x49, 0x49, 0x49, 0x36,
    0x3E, 0x41, 0x41, 0x41, 0x22,
    0x7F, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x49, 0x49, 0x49, 0x41,
    0x7F, 0x09, 0x09, 0x09, 0x01,
    0x3E, 0x41, 0x41, 0x51, 0x73,
    0x7F, 0x08, 0x08, 0x08, 0x7F,
    0x00, 0x41, 0x7F, 0x41, 0x00,
    0x20, 0x40, 0x41, 0x3F, 0x01,
    0x7F, 0x08, 0x14, 0x22, 0x41,
    0x7F, 0x40, 0x40, 0x40, 0x40,
    0x7F, 0x02, 0x1C, 0x02, 0x7F,
    0x7F, 0x04, 0x08, 0x10, 0x7F,
    0x3E, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x09, 0x09, 0x09, 0x06,
    0x3E, 0x41, 0x51, 0x21, 0x5E,
    0x7F, 0x09, 0x19, 0x29, 0x46,
    0x26, 0x49, 0x49, 0x49, 0x32,
    0x03, 0x01, 0x7F, 0x01, 0x03,
    0x3F, 0x40, 0x40, 0x40, 0x3F,
    0x1F, 0x20, 0x40, 0x20, 0x1F,
    0x3F, 0x40, 0x38, 0x40, 0x3F,
    0x63, 0x14, 0x08, 0x14, 0x63,
    0x03, 0x04, 0x78, 0x04, 0x03,
    0x61, 0x59, 0x49, 0x4D, 0x43,
    0x00, 0x7F, 0x41, 0x41, 0x41,
    0x02, 0x04, 0x08, 0x10, 0x20,
    0x00, 0x41, 0x41, 0x41, 0x7F,
    0x04, 0x02, 0x01, 0x02, 0x04,
    0x40, 0x40, 0x40, 0x40, 0x40,
    0x00, 0x03, 0x07, 0x08, 0x00,
    0x20, 0x54, 0x54, 0x78, 0x40,
    0x7F, 0x28, 0x44, 0x44, 0x38,
    0x38, 0x44, 0x44, 0x44, 0x28,
    0x38, 0x44, 0x44, 0x28, 0x7F,
    0x38, 0x54, 0x54, 0x54, 0x18,
    0x00, 0x08, 0x7E, 0x09, 0x02,
    0x18, 0xA4, 0xA4, 0x9C, 0x78,
    0x7F, 0x08, 0x04, 0x04, 0x78,
    0x00, 0x44, 0x7D, 0x40, 0x00,
    0x20, 0x40, 0x40, 0x3D, 0x00,
    0x7F, 0x10, 0x28, 0x44, 0x00,
    0x00, 0x41, 0x7F, 0x40, 0x00,
    0x7C, 0x04, 0x78, 0x04, 0x78,
    0x7C, 0x08, 0x04, 0x04, 0x78,
    0x38, 0x44, 0x44, 0x44, 0x38,
    0xFC, 0x18, 0x24, 0x24, 0x18,
    0x18, 0x24, 0x24, 0x18, 0xFC,
    0x7C, 0x08, 0x04, 0x04, 0x08,
    0x48, 0x54, 0x54, 0x54, 0x24,
    0x04, 0x04, 0x3F, 0x44, 0x24,
    0x3C, 0x40, 0x40, 0x20, 0x7C,
    0x1C, 0x20, 0x40, 0x20, 0x1C,
    0x3C, 0x40, 0x30, 0x40, 0x3C,
    0x44, 0x28, 0x10, 0x28, 0x44,
    0x4C, 0x90, 0x90, 0x90, 0x7C,
    0x44, 0x64, 0x54, 0x4C, 0x44,
    0x00, 0x08, 0x36, 0x41, 0x00,
    0x00, 0x00, 0x77, 0x00, 0x00,
    0x00, 0x41, 0x36, 0x08, 0x00,
    0x02, 0x01, 0x02, 0x04, 0x02
};


// Referência de código fonte:
// https://circuitdigest.com/microcontroller-projects/i2c-communication-with-pic-microcontroller-pic16f877a

void I2C_Initialize() // I2C master.
{
    TRISC3 = 1;
    TRISC4 = 1;
    SSPCON = 0b00101000;
    SSPCON2 = 0x28;
    SSPADD = 0x13;
    SSPSTAT = 0x80;
}

//HOLD------------------------------------------------------------------
//Used to hold the execution of the device until the current I2C operation is completed. 
//We would have to check if the I2C operations have to be held before starting any new operation. 
//This can be done by checking the register SSPSTAT and SSPCON2. 
//The SSPSTAT contains the information about status of the I2C bus.

void I2C_Hold() // Aguardar, enquanto o I2C estiver ocupado.
{
    while ((SSPCON2 & 0b00011111) || (SSPSTAT & 0b00000100)); // Verifica os registos, sabe se I2C nao esta a ser utilizado
}

// BEGIN and END ----------------------------------------------------------
//Each time while we write or read any data using the I2C bus we should begin and End the I2C connection. 
//To begin a I2C communication we have to set the SEN bit and to end the communication we have to set the PEN status bit. 
//Before toggling any of these bits it we should also check if the I2C bus is busy by using the function I2C_Hold 

void I2C_Begin() // Começar a comunicação.
{
    I2C_Hold();
    SEN = 1; // Começar comunicação.
}

void I2C_End() // Terminar a comunicação.
{
    I2C_Hold();
    PEN = 1; // Terminar comunicação.
}

//WRITE--------------------------------------------------------------------
//The data that has to be written to the IIC bus is passed through the variable data. 
//This data is then loaded into the SSPBUF buffer register to send it over the I2C bus.

void I2C_Write(unsigned char data) // Dados do master/slave.
{
    I2C_Hold();
    SSPBUF = data;
}

void ssd1306_command(uint8_t c) { // Comandos para display.
    I2C_Begin();
    I2C_Write(_i2caddr);
    I2C_Write(0x00);
    I2C_Write(c);
    I2C_End();
}


// Referência de código fonte:
// https://github.com/adafruit/Adafruit_SSD1306/commit/1a844297564bf8df79addf20b873658dcf3b01ac

// Inicializar o display.

void SSD1306_Init(uint8_t vccstate, uint8_t i2caddr) {
    _vccstate = vccstate;
    _i2caddr = i2caddr;

    ssd1306_command(SSD1306_DISPLAYOFF); // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV); // 0xD5
    ssd1306_command(0x80); // the suggested ratio 0x80

    ssd1306_command(SSD1306_SETMULTIPLEX); // 0xA8
    ssd1306_command(SSD1306_LCDHEIGHT - 1);

    ssd1306_command(SSD1306_SETDISPLAYOFFSET); // 0xD3
    ssd1306_command(0x0); // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0
    ssd1306_command(SSD1306_CHARGEPUMP); // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC) {
        ssd1306_command(0x10);
    } else {
        ssd1306_command(0x14);
    }
    ssd1306_command(SSD1306_MEMORYMODE); // 0x20
    ssd1306_command(0x00); // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);


    ssd1306_command(SSD1306_SETCOMPINS); // 0xDA
    ssd1306_command(0x12);
    ssd1306_command(SSD1306_SETCONTRAST); // 0x81
    if (vccstate == SSD1306_EXTERNALVCC) {
        ssd1306_command(0x9F);
    } else {
        ssd1306_command(0xCF);
    }


    ssd1306_command(SSD1306_SETPRECHARGE); // 0xD9
    if (vccstate == SSD1306_EXTERNALVCC) {
        ssd1306_command(0x22);
    } else {
        ssd1306_command(0xF1);
    }
    ssd1306_command(SSD1306_SETVCOMDETECT); // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME); // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY); // 0xA6

    ssd1306_command(SSD1306_DISPLAYON); // Turn on display.
}

// Número de linhas (Y) e ponto inicial da mensagem (X).

void SSD1306_GotoXY(uint8_t x, uint8_t y) {
    if ((x > SSD1306_LCDWIDTH / 6) || y > SSD1306_LCDHEIGHT / 8)
        return;
    x_pos = x;
    y_pos = y;
}

void SSD1306_PutC(char c) { // Escrever um caractere no display.
    uint8_t i, font_c;
    if ((c < ' ') || (c > '~'))
        c = '?';

    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0x00); //Column star address
    ssd1306_command(127); //Column end address

    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0x00); //Page start address 
    ssd1306_command(7); //Page end address

    I2C_Begin();
    I2C_Write(_i2caddr);
    I2C_Write(0x40);

    for (i = 0; i < 5; i++) {
        font_c = Font[(c - 32) * 5 + i];

        I2C_Write(font_c);
    }
    I2C_End();

    x_pos = x_pos % 21 + 1;
    if (x_pos == 1)
        y_pos = y_pos % 8 + 1;
}

void SSD1306_ClearDisplay() { // Limpar o display.
    uint16_t i;
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0); // Column start address
    ssd1306_command(127); // Column end address

    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0); // Page start address (0 = reset)
    ssd1306_command(7); // Page end address

    I2C_Begin();
    I2C_Write(_i2caddr);
    I2C_Write(0x40);

    for (i = 0; i < SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8; i++)
        I2C_Write(0);
    I2C_End();

}

void SSD1306_Print(char *s) { // Escrever a mensagem no display.
    uint8_t i = 0;
    while (s[i] != '\0') {
        if (s[i] == ' ' & x_pos == 1)
            i++;
        else
            SSD1306_PutC(s[i++]);
    }
}

void main() {
    initADC(); // Inicializar ADC.

    I2C_Initialize(); // Inicializar o I2C como master.
    SSD1306_Init(SSD1306_SWITCHCAPVCC, 0x78); // Inicializar o display I2C.

    while (1) {
        unsigned short ADC_port_0 = readADC(0);
        unsigned short ADC_port_1 = readADC(1);
        unsigned short ADC_port_2 = readADC(2);

        unsigned int ADC_result = ((((float) (ADC_port_0 + ADC_port_1 + ADC_port_2) / 3) / 1023)*5)*1000;

        unsigned char out_value[7];
        out_value[0] = (ADC_result / 10000) + 48; // Dezenas de milhar (dezenas).
        out_value[1] = (ADC_result / 1000) + 48; // Milhares (unidades).
        out_value[2] = '.';
        out_value[3] = ((ADC_result / 100) % 10) + 48; // Centenas (décimas).
        out_value[4] = ((ADC_result / 10) % 10) + 48; // Dezenas (centésimas).
        out_value[5] = (ADC_result % 10) + 48; // Unidades (milésimas).
        out_value[6] = '\0';

        SSD1306_ClearDisplay(); // Limpar o display I2C.

        SSD1306_GotoXY(2, 2); // Número de linhas (Y) e ponto inicial da mensagem (X).

        SSD1306_Print(out_value); // Print no display I2C.

        __delay_ms(2000);
    }
}
