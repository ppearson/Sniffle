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
#include <cctype> // for isdigit()

#include "utils/file_helpers.h"

Config::Config() :
	m_grepThreads(1),
	m_printProgressWhenOutToStdOut(true),
	m_directoryRecursionDepth(10),
	m_ignoreHiddenFiles(true),
	m_ignoreHiddenDirectories(true),
	m_preEmptiveSymlinkSkipping(true),
	m_matchCount(-1),
	m_flushOutput(true),
	m_outputFilename(true),
	m_outputContentLines(true),
	m_outputLineNumbers(false),
	m_outputFileSize(false),
	m_beforeLines(0),
	m_afterLines(0),
	m_blankLinesBetweenFiles(true),
	m_matchItemOrSeperatorChar('|'),
	m_matchItemAndSeperatorChar('&')
{

}

void Config::loadConfigFile()
{
	const char* homeDir = getenv("HOME");
	if (!homeDir)
		return;

	std::string configFilePath = FileHelpers::combinePaths(std::string(homeDir), ".config/sniffle.conf");

	std::fstream fileStream(configFilePath.c_str(), std::ios::in);
	if (!fileStream.is_open() || fileStream.fail())
	{
		return;
	}

	std::string line;
	char buf[1024];

	std::string key;
	std::string value;

	while (fileStream.getline(buf, 1024))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		line.assign(buf);

		if (getKeyValue(line, key, value))
		{
			applyKeyValueSetting(key, value);
		}
	}

	fileStream.close();
}

// this is pretty nasty in terms of needing to be coordinated with what's going on with the args parsing
// in main.cpp, but it means we can have all config related stuff in this file, which makes keeping the config
// file and args parsing in sync much easier...
// Although it means keeping this and the printHelp() output in sync is harder, so...
Config::ParseResult Config::parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex)
{
	int lastProcessedArg = startOptionArg;

	// TODO: use getopt()?
	bool handled = false;

	for (int i = startOptionArg; i < argc; i++)
	{
		if (argv[i][0] != '-')
		{
			continue;
		}

		handled = true;

		lastProcessedArg ++;

		// otherwise, we want to process it...
		std::string argString(argv[i]);

		if (argString.find("help") != std::string::npos)
		{
			return eParseHelpWanted;
		}
		else if (argString.size() > 4 && argString.substr(0, 2) == "--")
		{
			// handle full options
			std::string settingString = argString.substr(2);

			std::string key;
			std::string value;

			if (!getKeyValueFromArg(settingString, key, value))
			{
				fprintf(stderr, "Error: Malformed command line arg: %s\n", argString.c_str());
				return eParseError;
			}

			if (!applyKeyValueSetting(key, value))
			{
				fprintf(stderr, "Error: Unrecognised command line setting: %s\n", key.c_str());
				return eParseError;
			}
		}
		// handle some common short-hand conveniences...
		else if (argString == "-m")
		{
			std::string nextArg(argv[i + 1]);
			m_matchCount = atoi(nextArg.c_str());

			lastProcessedArg ++;
		}
		else if (argString == "-firstOnly")
		{
			m_matchCount = 1;
		}
		else if (argString == "-C")
		{
			std::string nextArg(argv[i + 1]);
			int contextLines = atoi(nextArg.c_str());

			m_afterLines = contextLines;
			m_beforeLines = contextLines;

			lastProcessedArg ++;
		}
		else if (argString == "-B")
		{
			std::string nextArg(argv[i + 1]);
			int contextLines = atoi(nextArg.c_str());

			m_beforeLines = contextLines;

			lastProcessedArg ++;
		}
		else if (argString == "-A")
		{
			std::string nextArg(argv[i + 1]);
			int contextLines = atoi(nextArg.c_str());

			m_afterLines = contextLines;

			lastProcessedArg ++;
		}

	}

	nextArgIndex = lastProcessedArg;

	if (handled)
	{
		return eParseHandledOK;
	}
	else
	{
		return eParseNoneHandled;
	}
}

// for config file
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

// for command line args string
bool Config::getKeyValueFromArg(const std::string& argString, std::string& key, std::string& value)
{
	size_t sepPos = argString.find("=");
	if (sepPos == std::string::npos)
		return false;

	key = argString.substr(0, sepPos);
	value = argString.substr(sepPos + 1);

	return true;
}

bool Config::applyKeyValueSetting(const std::string& key, const std::string& value)
{
	if (key == "grepThreads")
	{
		unsigned int intValue = atoi(value.c_str());
		m_grepThreads = intValue;
	}
	else if (key == "printProgressWhenOutToStdOut")
	{
		m_printProgressWhenOutToStdOut = getBooleanValueFromString(value);
	}
	else if (key == "directoryRecursionDepth")
	{
		m_directoryRecursionDepth = atoi(value.c_str());
	}
	else if (key == "ignoreHiddenFiles")
	{
		m_ignoreHiddenFiles = getBooleanValueFromString(value);
	}
	else if (key == "ignoreHiddenDirectories")
	{
		m_ignoreHiddenDirectories = getBooleanValueFromString(value);
	}
	else if (key == "preEmptiveSymlinkSkipping")
	{
		m_preEmptiveSymlinkSkipping = getBooleanValueFromString(value);
	}
	else if (key == "matchCount")
	{
		unsigned int intValue = atoi(value.c_str());
		m_matchCount = intValue;
	}
	else if (key == "flushOutput")
	{
		m_flushOutput = getBooleanValueFromString(value);
	}
	else if (key == "outputFilename")
	{
		m_outputFilename = getBooleanValueFromString(value);
	}
	else if (key == "outputContentLines")
	{
		m_outputContentLines = getBooleanValueFromString(value);
	}
	else if (key == "outputLineNumbers")
	{
		m_outputLineNumbers = getBooleanValueFromString(value);
	}
	else if (key == "blankLinesBetweenFiles")
	{
		m_blankLinesBetweenFiles = getBooleanValueFromString(value);
	}
	else if (key == "context")
	{
		unsigned int intValue = atoi(value.c_str());
		m_afterLines = intValue;
		m_beforeLines = intValue;
	}
	else if (key == "after-context")
	{
		unsigned int intValue = atoi(value.c_str());
		m_afterLines = intValue;
	}
	else if (key == "before-context")
	{
		unsigned int intValue = atoi(value.c_str());
		m_beforeLines = intValue;
	}
	else
	{
		return false;
	}

	return true;
}

bool Config::getBooleanValueFromString(const std::string& value)
{
	if (isdigit(value[0]))
	{
		// it's a digit, so...
		return atoi(value.c_str()) == 1;
	}
	else
	{
		// it's probably a string of some sort
		bool isTrue = value.find("true") != std::string::npos ||
					  value.find("yes") != std::string::npos ||
					  value.find("on") != std::string::npos;

		return isTrue;
	}
}
