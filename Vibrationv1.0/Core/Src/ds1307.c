/*
 * ds1307.c
 *
 *  Created on: Apr 11, 2024
 *      Author: artse
 */


#include "ds1307.h"
#include <stdio.h>

I2C_HandleTypeDef *ds1307_i2c;


/**
 * @brief Initialization of DS1307 module. Call this function before start working with RTC.
 * @param hi2c - I2C handler
 * @return 0 - OK, 1 - param = NULL
 */
uint8_t DS1307_Init(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == NULL)
		return 1;
	ds1307_i2c = hi2c;
	DS1307_SetClockHalt(0);
	return 0;
}

uint8_t DS1307_GetRegData(uint8_t addr)
{
	uint8_t value;
	HAL_I2C_Master_Transmit(ds1307_i2c, DS1307_ADDRESS << 1, &addr, 1, DS1307_TIMEOUT);
	HAL_I2C_Master_Receive(ds1307_i2c, DS1307_ADDRESS << 1, &value, 1, DS1307_TIMEOUT);
	return value;
}

void DS1307_SetRegData(uint8_t addr, uint8_t value)
{
	uint8_t bytes[2] = {addr, value};
	HAL_I2C_Master_Transmit(ds1307_i2c, DS1307_ADDRESS << 1, bytes, 2, DS1307_TIMEOUT);
}

uint8_t DS1307_GetClockHalt(void)
{
	return (DS1307_GetRegData(DS1307_REG_SECOND) & 0x80) >> 7;
}

void DS1307_SetClockHalt(uint8_t halt)
{
	uint8_t clockHalt = (halt ? 1 << 7 : 0);
	DS1307_SetRegData(DS1307_REG_SECOND, clockHalt | (DS1307_GetRegData(DS1307_REG_SECOND) & 0x7F));
}

void DS1307_SetRate(DS1307_Rate rate)
{
	uint8_t controlRegister = DS1307_GetRegData(DS1307_REG_CONTROL);
	uint8_t updatedControlRegister = (controlRegister & ~0x03) | rate;
	DS1307_SetRegData(DS1307_REG_CONTROL, updatedControlRegister);
}

uint8_t DS1307_GetSecond(void)
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_REG_SECOND));
}

void DS1307_SetSecond(uint8_t second)
{
	DS1307_SetRegData(DS1307_REG_SECOND, DS1307_EncodeBCD(second));
}

uint8_t DS1307_GetMinute(void)
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_REG_MINUTE));
}

void DS1307_SetMinute(uint8_t minute)
{
	DS1307_SetRegData(DS1307_REG_MINUTE, DS1307_EncodeBCD(minute));
}

uint8_t DS1307_GetHour(void)
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_REG_HOUR));
}

void DS1307_SetHour(uint8_t hour)
{
	DS1307_SetRegData(DS1307_REG_HOUR, DS1307_EncodeBCD(hour));
}

uint8_t DS1307_GetDOW(void)
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_REG_DOW));
}

void DS1307_SetDOW(uint8_t dow)
{
	DS1307_SetRegData(DS1307_REG_DOW, DS1307_EncodeBCD(dow));
}

uint8_t DS1307_GetDOM(void)
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_REG_DOM));
}

void DS1307_SetDOM(uint8_t dom)
{
	DS1307_SetRegData(DS1307_REG_DOM, DS1307_EncodeBCD(dom));
}

uint8_t DS1307_GetMonth(void)
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_REG_MONTH));
}

void DS1307_SetMonth(uint8_t month)
{
	DS1307_SetRegData(DS1307_REG_MONTH, DS1307_EncodeBCD(month));
}

uint8_t DS1307_GetYear(void)
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_REG_YEAR));
}

void DS1307_SetYear(uint8_t year)
{
	DS1307_SetRegData(DS1307_REG_YEAR, DS1307_EncodeBCD(year));
}

uint8_t DS1307_EncodeBCD(uint8_t dec)
{
	return (dec % 10 + ((dec / 10) << 4));
}

uint8_t DS1307_DecodeBCD(uint8_t bin)
{
	return (((bin & 0xF0) >> 4) * 10) + (bin & 0x0F);
}

void DS1307_SetDateTime(char time[])
{
	int year, month, day, hour, minute, second;
	sscanf(time, "%2d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);
	DS1307_SetYear(year);
	DS1307_SetMonth(month);
	DS1307_SetDOM(day);
	DS1307_SetHour(hour);
	DS1307_SetMinute(minute);
	DS1307_SetSecond(second);
}

char* DS1307_GetCurrentDateTimeCommand()
{
	int year, month, day, hour, minute, second;
	char* time = (char*)malloc(22 * sizeof(char));
	if (time == NULL) {
		return NULL;
	}
	year = DS1307_GetYear();
	month = DS1307_GetMonth();
	day = DS1307_GetDOM();
	hour = DS1307_GetHour();
	minute = DS1307_GetMinute();
	second = DS1307_GetSecond();
	sprintf(time, "(TIME,2,%02d%02d%02d%02d%02d%02d)", year, month, day, hour, minute, second);
	return time;
}

char* DS1307_GetCurrentDateTime()
{
	int year, month, day, hour, minute, second;
	char * time = (char*)malloc(13 * sizeof(char));
	if (time == NULL) {
		return NULL;
	}
	year = DS1307_GetYear();
	month = DS1307_GetMonth();
	day = DS1307_GetDOM();
	hour = DS1307_GetHour();
	minute = DS1307_GetMinute();
	second = DS1307_GetSecond();
	sprintf(time, "%02d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);
	time[13] = '\0';
	return time;
}

char* DS1307_GetCurrentDateTimeForFile()
{
	int day, hour, minute, second;
	char* time = (char*)malloc(9 * sizeof(char));
	if (time == NULL) {
		return NULL;
	}
	day = DS1307_GetDOM();
	hour = DS1307_GetHour();
	minute = DS1307_GetMinute();
	second = DS1307_GetSecond();
	sprintf(time, "%02d%02d%02d%02d", day, hour, minute, second);
	time[8] = '\0';
	return time;
}

// --------------- LAUNCH TIME ------------------
void DS1307_SetLaunchSecond(uint8_t second)
{
	DS1307_SetRegData(DS1307_ADDR_LAUNCH_SECOND, DS1307_EncodeBCD(second));
}

uint8_t DS1307_GetLaunchSecond()
{

	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_ADDR_LAUNCH_SECOND));
}

void DS1307_SetLaunchMinute(uint8_t minute)
{
	DS1307_SetRegData(DS1307_ADDR_LAUNCH_MINUTE, DS1307_EncodeBCD(minute));
}

uint8_t DS1307_GetLaunchMinute()
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_ADDR_LAUNCH_MINUTE));
}

void DS1307_SetLaunchHour(uint8_t hour)
{
	DS1307_SetRegData(DS1307_ADDR_LAUNCH_HOUR, DS1307_EncodeBCD(hour));
}

uint8_t DS1307_GetLaunchHour()
{
	return DS1307_DeocdeBCD(DS1307_GetRegData(DS1307_ADDR_LAUNCH_HOUR));
}

void DS1307_SetLaunchDOM(uint8_t dom)
{
	DS1307_SetRegData(DS1307_ADDR_LAUNCH_DOM, DS1307_EncodeBCD(dom));
}

uint8_t DS1307_GetLaunchDOM()
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_ADDR_LAUNCH_DOM));
}

void DS1307_SetLaunchMonth(uint8_t month)
{
	DS1307_SetRegData(DS1307_ADDR_LAUNCH_MONTH, DS1307_EncodeBCD(month));
}

uint8_t DS1307_GetLaunchMonth()
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_ADDR_LAUNCH_MONTH));
}

void DS1307_SetLaunchYear(uint8_t year)
{
	DS1307_SetRegData(DS1307_ADDR_LAUNCH_YEAR, DS1307_EncodeBCD(year));
}

uint8_t DS1307_GetLaunchYear()
{
	return DS1307_DecodeBCD(DS1307_GetRegData(DS1307_ADDR_LAUNCH_YEAR));
}
