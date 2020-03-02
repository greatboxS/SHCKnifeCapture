#ifndef arduino_json_extension_h
#define arduino_json_extention_h

#include "ArduinoJson.h"

template<typename T>
static void JsonParse_Element(JsonObject& _jsondoc, const char* _propertyName, T& _var)
{
	_var = _jsondoc.getMember(_propertyName).as<T>();
}

template<typename T>
static void JsonParse_Element(JsonObject& _jsondoc, const char* _propertyName, T* _var, size_t _len)
{
	const char* str = _jsondoc.getMember(_propertyName).as<const char*>();
	if (str != NULL)
		memccpy(_var, str, 0, _len);
}
//
template<typename T>
static void JsonParse_Element(JsonDocument& _jsondoc, const char* _propertyName, T& _var)
{
	_var = _jsondoc.getMember(_propertyName).as<T>();
}

template<typename T>
static void JsonParse_Element(JsonDocument& _jsondoc, const char* _propertyName, T* _var, size_t _len)
{
	const char* str = _jsondoc.getMember(_propertyName).as<const char*>();
	if (str != NULL)
		memccpy(_var, str, 0, _len);
}

template<typename T>
static void JsonParse_Element(JsonArray& _arr, uint8_t _index, const char* _propertyName, T& _var)
{
	_var = _arr[_index].getMember(_propertyName).as<T>();
}

template<typename T>
static void JsonParse_Element(JsonArray& _arr, uint8_t _index, const char* _propertyName, T* _var, size_t _len)
{
	const char* str = _arr[_index].getMember(_propertyName).as<const char*>();
	if (str != NULL)
		memccpy(_var, str, 0, _len);
}

#endif