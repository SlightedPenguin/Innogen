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

//ConfigManager configMgr(1);

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
	ConfigManager configMgr;

	// Start by reading the config
	configMgr.readConfig("queues.conf");

	// Connect to the service bus
	pn_messenger_t * messenger;
	messenger = pn_messenger(NULL);

	printf("Starting messenger... ");

	pn_messenger_start(messenger);

	printf("Done");

	string strNs = configMgr.getNamespace();
	string policy = configMgr.getPolicy();
	string queue = configMgr.getQueueName();
	string secret = configMgr.getSecret();

	string address = string("amqps://");
	address += policy;
	address += ":" + secret + "@" + strNs + "/" + queue;

	printf(" HERE: %s\r\n", address.c_str());

	pn_message_t * message;
	message = pn_message();
	pn_message_set_address(message, address.c_str());

	pn_data_t *body = pn_message_body(message);

	char buffer[100];
	// <message_format>/<from_node>/<timestmp>/<data_type>/<sensor_type>/<value>
	const char *fmt = "%d/%d/%lu/%d/%d/%s";

	snprintf(buffer, 100, fmt,
		0, // message_format
		1, // from_name
		1414792775, // timestamp
		2, // data_type
		3, // sensor_type
		"123123" // value
	);

	pn_data_put_string(body, pn_bytes(strlen(buffer), buffer));
	pn_messenger_put(messenger, message);
	check(messenger);

	pn_messenger_send(messenger, -1);
	check(messenger);

	pn_messenger_stop(messenger);
	pn_messenger_free(messenger);
	pn_message_free(message);

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

