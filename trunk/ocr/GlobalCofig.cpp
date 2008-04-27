#include "GlobalCofig.h"

const char *GlobalCofig::s_configFilePath = "data/config";

ConfigFile *GlobalCofig::s_config = NULL;

ConfigFile *GlobalCofig::getConfigFile()
{
	if(s_config == NULL){
		s_config = ConfigFile::parseConfig(s_configFilePath);
	}

	return s_config;
}