#include <time.h>
#include <stdio.h>
#include <ctime>
#include <RF24Network/RF24Network.h>
#include <RF24/RF24.h>
#include <iostream>
#include <cstdlib>

// MessageTypes.h

#ifndef _MESSAGETYPES_h
#define _MESSAGETYPES_h

typedef enum
{
	SensorTypeUndefined = 0x00,
	SensorTypeDHT11 = 0x01,
	SensorTypeDHT22 = 0x02,
	SensorTypeMCP9808 = 0x03,
	SensorTypeLM35 = 0x04
} TempSensorType;

typedef enum
{
	MsgTypeUndefined = 0x00,
	MsgTypeTemp = 0x01,
	MsgTypeTempReq = 0x02,
	MsgTypeTempResp = 0x03,
	MsgTypeTimeReq = 0x04,
	MsgTypeTimeResp = 0x05,
} MessageType;

struct TemperatureReading
{
	uint16_t value;
	uint8_t sensorType;
	uint8_t reserved;

	TemperatureReading(uint8_t sType, uint16_t v)
		: value(v), sensorType(sType)
	{
	}

	TemperatureReading(void)
		: value(0), sensorType(0)
	{
	}
};

typedef enum
{
	TempSourceUpstairs = 0x01,
	TempSourceDownstairs = 0x02,
	TempSourceOutside = 0x03,
} TempRequestSource;

struct TempRequest
{
	uint16_t tempSource;
	uint16_t reserved;
};

struct TimeResponse
{
	uint32_t seconds;
};


#endif

