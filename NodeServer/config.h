// config.h

#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>
#include <iostream>
#include <cstdlib>

using namespace std;

class ConfigManager
{
public:
	ConfigManager();
	void readConfig(const char* filename);
	//char* getNamespace();

	string getNamespace();
	string getQueueName();
	string getPolicy();
	string getSecret();

private:
	//char* strNamespace;
	bool isInitialized;

	string strNamespace;
	string dataIngressQueue;
	string dataIngressPolicy;
	string dataIngressSecret;
};

#endif

