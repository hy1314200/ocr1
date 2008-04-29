#ifndef _GLOBALCONFIG_H
#define _GLOBALCONFIG_H

#include "ConfigFile.h"

using namespace util;

class GlobalCofig
{
public:
	static ConfigFile *getConfigFile();

	static void releaseConfig(){
		if(s_config != 0){
			delete s_config;
			s_config = 0;
		}
	}

private:
	static const char *s_configFilePath;

	static ConfigFile *s_config;

	GlobalCofig(){}
	~GlobalCofig(void){}
};

#endif
