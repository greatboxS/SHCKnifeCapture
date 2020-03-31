#ifndef global_config_h
#define global_config_h
#include <Arduino.h>

#define EEPROM_MAX_SIZE 2048
#define MACHINE_MAX_NAME_LENGTH 16

#define MACHINE_START_ADDR 64 // total + machine +machine ...
#define LOCAL_DEVICE_ID_ADDR 0
#define ETHERNET_START_ADDR 4
// #define SERVER_PORT_ADDR 24
// #define DEVICE_MAC_ADDR 28 //28 +6 //34

#ifndef function_log(...)
#define function_log(...) printf("\r\n[USER_DEBUG] ----> Func: %s at line: %d\r\n", __func__, __LINE__)
#endif

#define MAX_QUEUE_LENGTH 10
#define NEX_BUT_SETTING 0xB0
#define NEX_BUT_NEXT 0xB1
#define NEX_BUT_BACK 0xB2

#define NEX_BUT_1_NEW_LEFT 0x00
#define NEX_BUT_1_OLD_LEFT 0x02
#define NEX_BUT_1_NEW_RIGHT 0x01
#define NEX_BUT_1_OLD_RIGHT 0x03

#define NEX_BUT_2_NEW_LEFT 0x04
#define NEX_BUT_2_OLD_LEFT 0x06
#define NEX_BUT_2_NEW_RIGHT 0x05
#define NEX_BUT_2_OLD_RIGHT 0x07

#define NEX_BUT_3_NEW_LEFT 0x08
#define NEX_BUT_3_OLD_LEFT 0x0A
#define NEX_BUT_3_NEW_RIGHT 0x09
#define NEX_BUT_3_OLD_RIGHT 0x0B

#define NEX_PAGE_INIT 0xD0

#define BUT_SAVE_MACHINE_NAME 0xA0
#define BUT_SAVE_TOTAL_MACHINE 0xA1
#define BUT_SAVE_LOCAL_DEVICE_ID 0xA2
#define BUT_RESET_MODULE 0xA3
#define BUT_GOTO_SETTING2 0xA4

#define BUT_SAVE_MAC 0xA5
#define BUT_SAVE_SERVER_IP 0xA6
#define BUT_RESET_MACHINE_COUNTER 0xAA

#define BUT_OK 0xC0
#define BUT_NO 0xC1

#define ETHERNET_CS_PIN GPIO_NUM_5
#define ETHERNET_RST_PIN GPIO_NUM_26

#define EX_QUEUE_FULL 0
#define EX_SUCCESS 1
#define EX_CREATE_FAIL -1

#define MAX_MACHINE_NUM 10
#define MAX_MACHINE_PER_PAGE 3

typedef struct
{
	char code[32]; //code
	int dataLength;
	char buf[1024 * 3];
} http_header_t;

#endif