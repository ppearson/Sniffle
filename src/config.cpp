/*
 Sniffle
 Copyright 2018 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <fstream>

#include "file_helpers.h"

Config::Config() : 
    m_grepThreads(1),
	m_firstResultOnly(false),
	m_outputFilename(true),
	m_outputContentLines(true),
	m_outputLineNumbers(false),
	m_beforeLines(0),
	m_afterLines(0),
    m_blankLinesBetweenFiles(false)
{
	
}

void Config::loadConfigFile()
{
	const char* homeDir = getenv("HOME");
	if (!homeDir)
		return;
	
	std::string configFilePath = FileHelpers::combinePaths(std::string(homeDir), ".config/sniffle.conf");
	
	std::fstream fileStream(configFilePath.c_str(), std::ios::in);
	if (fileStream.fail())
	{
		return;
	}

	std::string line;
	char buf[1024];

	std::string key;
	std::string value;

	while (fileStream.getline(buf, 1024))
	{
		line.assign(buf);

		if (buf[0] == 0 || buf[0] == '#')
			continue;

		if (getKeyValue(line, key, value))
		{
			if (key == "grepThreads")
			{
				unsigned int intValue = atoi(value.c_str());
				m_grepThreads = intValue;
			}
			else if (key == "firstResultOnly")
			{
				unsigned int intValue = atoi(value.c_str());
				m_firstResultOnly = intValue == 1;
			}
			else if (key == "outputFilename")
			{
				unsigned int intValue = atoi(value.c_str());
				m_outputFilename = intValue == 1;
			}
			else if (key == "outputContentLines")
			{
				unsigned int intValue = atoi(value.c_str());
				m_outputContentLines = intValue == 1;
			}
			else if (key == "outputLineNumbers")
			{
				unsigned int intValue = atoi(value.c_str());
				m_outputLineNumbers = intValue == 1;
			}
			else if (key == "blankLinesBetweenFiles")
			{
				unsigned int intValue = atoi(value.c_str());
				m_blankLinesBetweenFiles = intValue == 1;
			}
			else if (key == "contextLines")
			{
				unsigned int intValue = atoi(value.c_str());
				m_afterLines = intValue;
				m_beforeLines = intValue;
			}
		}
	}

	fileStream.close();
}

// this is pretty nasty in terms of needing to be coordinated with what's going on with the args parsing
// in main.cpp, but it means we can have all config related stuff in this file, which makes keeping the config
// file and args parsing in sync much easier...
// Although it means keeping this and the printHelp() output in sync is harder, so...
bool Config::parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex)
{
	for (int i = startOptionArg; i < argc; i++)
	{
		
	}
	return true;
}

bool Config::getKeyValue(const std::string& configLine, std::string& key, std::string& value)
{
	size_t sepPos = configLine.find(":");
	if (sepPos == std::string::npos)
		return false;

	size_t valueStart = configLine.find_first_not_of(" ", sepPos + 1);
	if (valueStart == std::string::npos)
		return false;

	key = configLine.substr(0, sepPos);
	value = configLine.substr(valueStart);

	return true;
}