/*
 * command_vibric.c
 *
 *  Created on: Apr 30, 2024
 *      Author: artse
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command_viric.h"

char* Vibric_ParseCommandName(char request[])
{
	char* command = (char*)malloc(5 * sizeof(char));
	if (command == NULL) {
		return NULL;
	}
	memcpy(command, &request[1], 4);
	command[4] = '\0';
	return command;
}

int Vibric_ParseCommandCode(char request[])
{
	int code;
	sscanf(request, "(%*[^,],%1d", &code);
	return code;
}

char* Vibric_ParseCommandData(char request[])
{
	char* data = (char*)malloc(13 * sizeof(char));
	if (data == NULL)
		return NULL;
	sscanf(request, "(%*[^,],%*d,%s)", data);
	data[12] = '\0';
	return data;
}

int Vibric_ConvertCommandDataToInt(char data_to_convert[])
{
	int data;
	sscanf(data_to_convert, "%d", &data);
	return data;
}


Vibric_CommandTypeDef Vibric_ParseCommand(char request[])
{
	Vibric_CommandTypeDef command;
	command.name = Vibric_ParseCommandName(request);
	command.code = Vibric_ParseCommandCode(request);
	if ((command.code % 2) == 1)
		command.data = Vibric_ParseCommandData(request);
	return command;
}
