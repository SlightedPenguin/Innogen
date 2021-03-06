#include <cstdlib>
#include <iostream>
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <proton/message.h>
#include <proton/messenger.h>
#include <ctime>
#include <stdio.h>
#include <time.h>
#include "sensornet.h"
#include "config.h"
#include "MessageTypes.h"

#define check(messenger)                                                     \
  {                                                                          \
    if(pn_messenger_errno(messenger))                                        \
    {                                                                        \
      die(__FILE__, __LINE__, pn_error_text(pn_messenger_error(messenger))); \
    }                                                                        \
  }                                                                          \

void die(const char *file, int line, const char *message)
{
  fprintf(stderr, "%s:%i: %s\n", file, line, message);
  exit(1);
}


using namespace std;

// Setup for GPIO 22 CE and CE1 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  

RF24Network network(radio);

ConfigManager configMgr;

pn_messenger_t * messenger;
string address;

// /<message_format>/<from_node>/<timestmp>/<data_type>/<sensor_type>/<value>
const char *messageFmt = "/%d/%u/%u/%d/%d/%s";

// Address of our node in Octal format
const uint16_t this_node = 00;

float unpack8dot8(uint16_t d)
{
	float f = d >> 8;
	f += (float)(d & 0xFF) / 256;
	return f;
}

void send_sensor_data(int node, int sensorId, int dataType, int sensorType, string value)
{
	pn_message_t * message;
	message = pn_message();
	pn_message_set_address(message, address.c_str());

	pn_data_t *body = pn_message_body(message);

	char buffer[100];

	uint32_t fullSensorId = node;
	fullSensorId <<= 16;
	fullSensorId |= sensorId; 

	time_t t;
    time(&t);

	snprintf(buffer, 100, messageFmt,
		1, // message_format
		fullSensorId, // sensorId
		(unsigned int)t, // timestamp
		dataType, // data_type
		sensorType, // sensor_type
		value.c_str() // value
	);

	pn_data_put_string(body, pn_bytes(strlen(buffer), buffer));
	pn_messenger_put(messenger, message);
	check(messenger);

	pn_messenger_send(messenger, -1);
	check(messenger);

	//pn_messenger_stop(messenger);
	//pn_messenger_free(messenger);
	pn_message_free(message);
}

void handle_temp_send(RF24NetworkHeader header)
{
	TemperatureReading payload;

	network.read(header,&payload,sizeof(payload));

	float c = unpack8dot8(payload.value);
	printf("Received payload with value 0x%04x from 0%o\n", payload.value, header.from_node);
	printf("  ==> 0x%02x\n", payload.sensorType);
	
	char strTemp[10];
	snprintf(strTemp, 10, "%f", c);
	printf("TEMP: %s\r\n", strTemp);
	
	send_sensor_data(header.from_node, 0, header.type, payload.sensorType, strTemp);
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
	// Start by reading the config
	configMgr.readConfig("queues.conf");

	// Connect to the service bus
	messenger = pn_messenger(NULL);

	printf("Starting messenger... ");
	pn_messenger_start(messenger);
	printf("Done");

	// Get the config parameter needed to send messages
	string strNs = configMgr.getNamespace();
	string policy = configMgr.getPolicy();
	string queue = configMgr.getQueueName();
	string secret = configMgr.getSecret();

	// Build the address
	address = string("amqps://");
	address += policy + ":" + secret + "@" + strNs + "/" + queue;

	// Setup the RF24 radio and network
	radio.begin();
	
	delay(5);
	network.begin(/*channel*/ 92, /*node address*/ this_node);
	radio.printDetails();

	// Main loop
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

