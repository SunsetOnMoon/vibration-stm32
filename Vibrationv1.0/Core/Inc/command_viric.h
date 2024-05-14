/*
 * command_vibric.h
 *
 *  Created on: Apr 30, 2024
 *      Author: artse
 */

#ifndef INC_COMMAND_VIRIC_H_
#define INC_COMMAND_VIRIC_H_

typedef struct {
	char* name;
	int code;
	char* data;
} Vibric_CommandTypeDef;


char* Vibric_ParseCommandName(char request[]);

int Vibric_ParseCommandCode(char request[]);

char* Vibric_ParseCommandData(char request[]);

int Vibric_ConvertCommandDataToInt(char data_to_convert[]);

Vibric_CommandTypeDef Vibric_ParseCommand(char request[]);


#endif /* INC_COMMAND_VIRIC_H_ */
