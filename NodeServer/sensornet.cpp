#include <cstdlib>
#include <iostream>
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <ctime>
#include <stdio.h>
#include <time.h>
#include "sensornet.h"
#include "MessageTypes.h"

// Setup for GPIO 22 CE and CE1 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  

RF24Network network(radio);

// Address of our node in Octal format
const uint16_t this_node = 00;

float unpack8dot8(uint16_t d)
{
	float f = d >> 8;
	f += (float)(d & 0xFF) / 256;
	return f;
}

void handle_temp_send(RF24NetworkHeader header)
{
	TemperatureReading payload;

	network.read(header,&payload,sizeof(payload));

	float c = unpack8dot8(payload.value);
	printf("Received payload with value 0x%04x from 0%o\n", payload.value, header.from_node);
	printf("  ==> 0x%02x\n", payload.sensorType);
	printf("TEMP: %f\r\n", c);
	
}

void handle_temp_req(RF24NetworkHeader header)
{
	TempRequest payload;

	network.read(header,&payload,sizeof(payload));
}

void handle_time_req(RF24NetworkHeader header)
{
	printf("handle_time_req() from node 0%o\r\n", header.from_node);

	time_t t;
	time(&t);
	printf("Time is %u\r\n", (unsigned int)t);
	
	TimeResponse response;
	response.seconds = (unsigned int)t;

	RF24NetworkHeader respHeader(header.from_node, 0x05);
	network.write(respHeader, &response, sizeof(TimeResponse));

}

int main(int argc, char** argv) 
{
	radio.begin();
	
	delay(5);
	network.begin(/*channel*/ 92, /*node address*/ this_node);
	radio.printDetails();

	while(1)
	{
		network.update();

		while (network.available())
		{
			RF24NetworkHeader header;

			network.peek(header);

			switch(header.type)
			{
				case 0x01:
					handle_temp_send(header);
					break;
				case 0x02:
					handle_temp_req(header);
					break;
				case 0x03:
					handle_time_req(header);
					break;
				default:
					printf(
						"%u: UNKNOWN TYPE type 0x%x from 0%o\n\r", 
						millis(), 
						header.type, 
						header.from_node);
					break;
			}
		}		  

		sleep(2);
	}

	return 0;
}

