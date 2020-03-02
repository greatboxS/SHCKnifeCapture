#ifndef nextion_extension_h
#define nextion_extension_h
#include <HardwareSerial.h>
#include "NextionVariableString.h"
#include "NextionVariableNumeric.h"
#include "NextionText.h"
#include "NextionPicture.h"
#include "NextionPage.h"
#include "NextionNumber.h"
#include "NextionButton.h"
#include "Nextion.h"
#include "INextionTouchable.h"

#define NextionSerial Serial2
const char *NexPropertyType[5] = {"txt", "val", "bco", "pic", "pco"};
Nextion nex(NextionSerial);
INextionTouchable PAGE_LOADING_EVENT = INextionTouchable(nex);

static void nex_listening()
{
	nex.poll();
}

static void sendCommand(char *commandStr)
{
	NextionSerial.print(commandStr);
	NextionSerial.write(0xFF);
	NextionSerial.write(0xFF);
	NextionSerial.write(0xFF);
	printf("nextion cmd: %s\r\n", commandStr);
}

static void nex_goto_page(char *page_name)
{
	size_t commandLen = 6 + strlen(page_name);
	char command[commandLen];
	snprintf(command, commandLen, "page %s", page_name);
	sendCommand(command);
}

static void getPropertyName(const char *inputName, char *outputName, uint8_t type)
{
	uint8_t propertyNameLen = strlen(inputName) + 24;
	snprintf(outputName, propertyNameLen, "%s.%s", inputName, NexPropertyType[type]);
}

static void setNumberProperty(const char *pageName, char *propertyName, uint32_t value)
{
	size_t commandLen = 8 + strlen(pageName) + strlen(propertyName);
	char commandBuffer[commandLen];
	snprintf(commandBuffer, commandLen, "%s.%s=%ld", pageName, propertyName, value);
	sendCommand(commandBuffer);
}

static uint32_t getNumberProperty(const char *pageName, char *propertyName)
{
	size_t commandLen = 7 + strlen(pageName) + strlen(propertyName);
	char commandBuffer[commandLen];
	snprintf(commandBuffer, commandLen, "get %s.%s", pageName, propertyName);
	sendCommand(commandBuffer);
	uint32_t id;
	if (nex.receiveNumber(&id))
		return id;
	else
		return 0;
}

static void setStringProperty(const char *pageName, char *propertyName, char *value)
{
	size_t commandLen = 7 + strlen(pageName) + strlen(propertyName) + strlen(value);
	char command[commandLen];
	snprintf(command, commandLen, "%s.%s=\"%s\"", pageName, propertyName, value);
	sendCommand(command);
}

static size_t getStringProperty(const char *pageName, char *propertyName, char *value, size_t len)
{
	size_t commandLen = 6 + strlen(pageName) + strlen(propertyName);
	char command[commandLen];
	snprintf(command, commandLen, "get %s.%s", pageName, propertyName);
	sendCommand(command);
	return nex.receiveString(value, len);
}

static void setNumberProperty(const char *propertyName, uint32_t value)
{
	size_t commandLen = 8 + strlen(propertyName);
	char commandBuffer[commandLen];
	snprintf(commandBuffer, commandLen, "%s=%ld", propertyName, value);
	sendCommand(commandBuffer);
}

static uint32_t getNumberProperty(const char *propertyName)
{
	size_t commandLen = 7 + strlen(propertyName);
	char commandBuffer[commandLen];
	snprintf(commandBuffer, commandLen, "get %s", propertyName);
	sendCommand(commandBuffer);
	uint32_t id;
	if (nex.receiveNumber(&id))
		return id;
	else
		return 0;
}

static void setStringProperty(const char *propertyName, char *value)
{
	size_t commandLen = 7 + strlen(propertyName) + strlen(value);
	char command[commandLen];
	snprintf(command, commandLen, "%s=\"%s\"", propertyName, value);
	sendCommand(command);
}

static size_t getStringProperty(const char *propertyName, char *value, size_t len)
{
	size_t commandLen = 6 + strlen(propertyName);
	char command[commandLen];
	snprintf(command, commandLen, "get %s", propertyName);
	sendCommand(command);
	return nex.receiveString(value, len);
}

static void nex_send_message(char *message)
{
	setStringProperty("SCREEN1", "msg.txt", message);
	setStringProperty("SETTING", "msg.txt", message);
	setStringProperty("SETTING2", "msg.txt", message);
}

static void nex_init()
{
	NextionSerial.begin(115200);
	printf("Initialize nextion screen");
	nex_goto_page("SCREEN1");
}

static void nex_set_page_number(int num)
{
	setNumberProperty("SCREEN1", "n12.val", num);
}
#endif
