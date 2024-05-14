/*
 * ds1307.h
 *
 *  Created on: Apr 11, 2024
 *      Author: artse
 */

#ifndef INC_DS1307_H_
#define INC_DS1307_H_

#include "stm32f1xx.h"
#include <stdlib.h>
#include <string.h>

//__________________________ADDRESSES________________________________
#define DS1307_ADDRESS_WRITE 0xD0
#define DS1307_ADDRESS_READ 0xD1
#define DS1307_ADDRESS 0x68
#define DS1307_REG_SECOND 0x00
#define DS1307_REG_MINUTE 0x01
#define DS1307_REG_HOUR 0x02
#define DS1307_REG_DOW 0x03 // DOW - day of week
#define DS1307_REG_DOM 0x04 // DOM - day of month (a.k.a date)
#define DS1307_REG_MONTH 0x05
#define DS1307_REG_YEAR 0x06
#define DS1307_REG_CONTROL 0x07

#define DS1307_ADDR_LAUNCH_YEAR 0x08
#define DS1307_ADDR_LAUNCH_MONTH 0x09
#define DS1307_ADDR_LAUNCH_DOM 0x10
#define DS1307_ADDR_LAUNCH_HOUR 0x11
#define DS1307_ADDR_LAUNCH_MINUTE 0x12
#define DS1307_ADDR_LAUNCH_SECOND 0x13

#define DS1307_ADDR_END_YEAR 0x14
#define DS1307_ADDR_END_MONTH 0x15
#define DS1307_ADDR_END_DOM 0x16
#define DS1307_ADDR_END_HOUR 0x17
#define DS1307_ADDR_END_MINUTE 0x18
#define DS1307_ADDR_END_SECOND 0x19
//___________________________________________________________________


//_____________________________FLAGS_________________________________
#define DS1307_FLAG_CH 0b10000000
#define DS1307_FLAG_24_MODE 0b01000000
//___________________________________________________________________

#define DS1307_TIMEOUT 5000

typedef enum DS1307_Rate {
	DS1307_1Hz,
	DS1307_4096Hz,
	DS1307_8192Hz,
	DS1307_32768Hz
} DS1307_Rate;

typedef enum DS1307_SQWE_Mode {
	DS1307_DISABLED,
	DS1307_ENABLED
} DS1307_SQWE_Mode;

typedef enum DS1307_HourMode {
	DS1307_24_HOUR,
	DS1307_12_HOUR = 0b01000000
} DS1307_HourMode;


uint8_t DS1307_Init(I2C_HandleTypeDef *hi2c);

uint8_t DS1307_GetRegData(uint8_t addr);
void DS1307_SetRegData(uint8_t addr, uint8_t value);

uint8_t DS1307_GetClockHalt(void);
void DS1307_SetClockHalt(uint8_t halt);

uint8_t DS1307_SetSQWEMode(DS1307_SQWE_Mode mode);
void DS1307_SetRate(DS1307_Rate rate);

uint8_t DS1307_GetSecond(void);
void DS1307_SetSecond(uint8_t second);

uint8_t DS1307_GetMinute(void);
void DS1307_SetMinute(uint8_t minute);

uint8_t DS1307_GetHour(void);
void DS1307_SetHour(uint8_t hour);

uint8_t DS1307_GetDOW(void);
void DS1307_SetDOW(uint8_t dow);

uint8_t DS1307_GetDOM(void);
void DS1307_SetDOM(uint8_t dom);

uint8_t DS1307_GetMonth(void);
void DS1307_SetMonth(uint8_t month);

uint8_t DS1307_GetYear(void);
void DS1307_SetYear(uint8_t year);

uint8_t DS1307_EncodeBCD(uint8_t dec);
uint8_t DS1307_DecodeBCD(uint8_t bin);

void DS1307_SetDateTime(char time[]);
char* DS1307_GetCurrentDateTime();
char* DS1307_GetCurrentDateTimeCommand();
char* DS1307_GetCurrentDateTimeForFile();

// ----------------- LAUNCH TIME -------------------------
void DS1307_SetLaunchTime(char time[]);
char* DS1307_GetLaunchTime();

void DS1307_SetLaunchSecond(uint8_t second);
uint8_t DS1307_GetLaunchSecond();

void DS1307_SetLaunchMinute(uint8_t minute);
uint8_t DS1307_GetLaunchMinute();

void DS1307_SetLaunchHour(uint8_t hour);
uint8_t DS1307_GetLaunchHour();

void DS1307_SetLaunchDOM(uint8_t dom);
uint8_t DS1307_GetLaunchDOM();

void DS1307_SetLaunchMonth(uint8_t month);
uint8_t DS1307_GetLaunchMonth();

void DS1307_SetLaunchYear(uint8_t year);
uint8_t DS1307_GetLaunchYear();

// --------------- END TIME -------------------------------
void DS1307_SetEndTime(char time[]);
char* DS1307_GetEndTime();

void DS1307_SetEndSecond(uint8_t second);
uint8_t DS1307_GetEndSecond();

void DS1307_SetEndMinute(uint8_t minute);
uint8_t DS1307_GetEndMinute();

void DS1307_SetEndHour(uint8_t hour);
uint8_t DS1307_GetEndHour();

void DS1307_SetEndDOM(uint8_t dom);
uint8_t DS1307_GetEndDOM();

void DS1307_SetEndMonth(uint8_t month);
uint8_t DS1307_GetEndMonth();

void DS1307_SetEndYear(uint8_t year);
uint8_t DS1307_GetEndYear();

#endif /* INC_DS1307_H_ */
