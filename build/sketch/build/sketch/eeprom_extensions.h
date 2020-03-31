#ifndef eeprom_extension_h
#define eeprom_extension_h
#include <EEPROM.h>

template <typename T>
void eeprom_write(T val, int addr)
{
	uint8_t size = sizeof(T);
	printf("Write %d bytes have value %d to address %d\r\n", size, val, addr);

	for (uint8_t i = size; i > 0; i--)
	{
		uint8_t value = 0;
		value = uint8_t(val >> ((i - 1) * 8));
		EEPROM.write(addr + (size - i), value);
		printf("byte index %d, val %d\r\n", i, value);
	}
}

template <typename T>
T eeprom_read(int addr)
{
	T result = 0;
	uint8_t size = sizeof(T);

	for (uint8_t i = size; i > 0; i--)
	{
		T v = (T)EEPROM.read(addr + (size - i));
		printf("byte index %d, val %d\r\n", i, v);
		
		result |= (T)(v << ((i - 1) * 8));
	}
	printf("Read %d bytes from address %d, return %d\r\n", sizeof(T), addr, result);
	return result;
}
#endif