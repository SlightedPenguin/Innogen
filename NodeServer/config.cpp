#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "config.h"

using namespace std;

ConfigManager::ConfigManager()
{
}

void ConfigManager::readConfig(const char* filename)
{
	printf("In readConfig(%s)\r\n", filename);

	string keyNamespace("namespace");
	string keyIngressQueue("sensor_data_queue");
	string keyIngressPolicy("sensor_data_policy");
	string keyIngressSecret("sensor_data_secret");

	string line;
	ifstream configFile(filename);

	if (configFile.is_open())
	{
		while(getline(configFile, line))
		{
			if (line.compare(0, 1, "#") == 0)
			{
				continue;
			}

			if (line.compare(0, keyNamespace.length(), keyNamespace) == 0)
			{
				strNamespace = line.substr(keyNamespace.length()+1,line.length() - (keyNamespace.length()+1));
			}

			if (line.compare(0, keyIngressQueue.length(), keyIngressQueue) == 0)
			{
				dataIngressQueue = line.substr(keyIngressQueue.length()+1,line.length() - (keyIngressQueue.length()+1));
			}

			if (line.compare(0, keyIngressPolicy.length(), keyIngressPolicy) == 0)
			{
				dataIngressPolicy = line.substr(keyIngressPolicy.length()+1,line.length() - (keyIngressPolicy.length()+1));
			}

			if (line.compare(0, keyIngressSecret.length(), keyIngressSecret) == 0)
			{
				dataIngressSecret = line.substr(keyIngressSecret.length()+1,line.length() - (keyIngressSecret.length()+1));
			}
		}
		configFile.close();
	}
	else
	{
		cout << "Unable to open file";
	}
}

string ConfigManager::getNamespace()
{
	return strNamespace;
}

string ConfigManager::getQueueName()
{
	return dataIngressQueue;
}

string ConfigManager::getPolicy()
{
	return dataIngressPolicy;
}

string ConfigManager::getSecret()
{
	return dataIngressSecret;
}

/*
char* getNamespace()
{
	return NULL;
}
*/
