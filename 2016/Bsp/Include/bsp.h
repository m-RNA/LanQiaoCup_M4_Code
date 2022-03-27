#ifndef __BSP_H_
#define __BSP_H_


#include "main.h"
#include "lcd.h"


void LED_Disp(u8 ucLED);

u8 KEY_Scan(void);

u16 ADC_GetVal(void);

void EEPROM_Write(u8 * Data, u8 Size, u8 Addr);

void EEPROM_Read(u8 * Data, u8 Size, u8 Addr);





#endif

