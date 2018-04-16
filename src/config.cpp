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
#include <string.h>
#include <fstream>
#include <cctype> // for isdigit()

#include "utils/file_helpers.h"

Config::Config() :
	m_findThreads(1),
	m_grepThreads(1),
	m_printProgressWhenOutToStdOut(true),
	m_directoryRecursionDepth(10),
	m_ignoreHiddenFiles(true),
	m_ignoreHiddenDirectories(true),
	m_followSymlinks(true),
	m_preEmptiveSymlinkSkipping(true),
	m_matchCount(-1),
	m_flushOutput(true),
	m_outputFilename(true),
	m_outputRelativeFilename(false),
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
			// TODO: this is really bad, but currently the only way to make this processing robust.
			//       This whole command line processing mess needs to be completely re-done at some point...
			if (strcmp(argv[i], "find") == 0 || strcmp(argv[i], "grep") == 0 || strcmp(argv[i], "match") == 0)
			{
				// we've reached the command to run, so we can break out.
				break;
			}
			continue;
		}
		
		std::string argString(argv[i]);
		
		// otherwise, we want to process it...
		
		handled = true;

		lastProcessedArg ++;

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
		else if (argString == "-ft")
		{
			// find threads
			
			std::string nextArg(argv[i + 1]);
			m_findThreads = atoi(nextArg.c_str());

			lastProcessedArg ++;
		}
		else if (argString == "-gt")
		{
			// grep threads
			
			std::string nextArg(argv[i + 1]);
			m_grepThreads = atoi(nextArg.c_str());

			lastProcessedArg ++;
		}
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
		else if (argString == "-n")
		{
			m_outputLineNumbers = true;
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
		else if (argString == "-rd")
		{
			std::string nextArg(argv[i + 1]);
			int recursionDepthLimit = atoi(nextArg.c_str());

			m_directoryRecursionDepth = recursionDepthLimit;

			lastProcessedArg ++;
		}
		else
		{
			// otherwise, don't do anything, as it might be a filter param
			// and we want other code to handle that (isn't great that this
			// type of thing is in two places, but...)
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

void Config::printFullOptions() const
{
	fprintf(stderr, "\nFull options: (specify with --<option>=<value> or in sniffle.conf file):\n");
	fprintf(stderr, "(showing defaults):\n");
	fprintf(stderr, "findThreads: %u: \n", m_findThreads);
//	fprintf(stderr, "grepThreads: %u: \n", m_grepThreads);
	fprintf(stderr, "printProgressWhenOutToStdOut: %i:\n", m_printProgressWhenOutToStdOut);
	fprintf(stderr, "directoryRecursionDepth: %i:\n", m_directoryRecursionDepth);
	fprintf(stderr, "ignoreHiddenFiles: %i:\n", m_ignoreHiddenFiles);
	fprintf(stderr, "ignoreHiddenDirectories: %i:\n", m_ignoreHiddenDirectories);
	fprintf(stderr, "followSymlinks: %i:\n", m_followSymlinks);
	fprintf(stderr, "preEmptiveSymlinkSkipping: %i:\n", m_preEmptiveSymlinkSkipping);
	fprintf(stderr, "matchCount: %i:\n", m_matchCount);
	fprintf(stderr, "flushOutput: %i:\n", m_flushOutput);
	fprintf(stderr, "outputFilename: %i:\t\tOutput the filename before matched results within file.\n", m_outputFilename);
	fprintf(stderr, "outputRelativeFilename: %i:\t\n", m_outputRelativeFilename);
	fprintf(stderr, "outputContentLines: %i:\n", m_outputContentLines);
	fprintf(stderr, "outputLineNumbers: %i:\n", m_outputLineNumbers);
	fprintf(stderr, "blankLinesBetweenFiles: %i:\n", m_blankLinesBetweenFiles);
	fprintf(stderr, "matchItemOrSeperatorChar: %c:\n", m_matchItemOrSeperatorChar);
	fprintf(stderr, "matchItemAndSeperatorChar: %c:\n", m_matchItemAndSeperatorChar);
	fprintf(stderr, "context:\t\t\tContent lines to print either side of match.\n");
	fprintf(stderr, "after-context:\t\t\tContent lines to print after match.\n");
//	fprintf(stderr, "before-context:\t\t\tContent lines to print before match.\n");
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
	if (key == "findThreads")
	{
		unsigned int intValue = atoi(value.c_str());
		m_findThreads = intValue;
	}
	else if (key == "grepThreads")
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
	else if (key == "followSymlinks")
	{
		m_followSymlinks = getBooleanValueFromString(value);
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
	else if (key == "outputRelativeFilename")
	{
		m_outputRelativeFilename = getBooleanValueFromString(value);
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
	else if (key == "matchItemOrSeperatorChar")
	{
		if (!value.empty())
		{
			m_matchItemOrSeperatorChar = value[0];
		}
	}
	else if (key == "matchItemAndSeperatorChar")
	{
		if (!value.empty())
		{
			m_matchItemAndSeperatorChar = value[0];
		}
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
