/*
 Sniffle
 Copyright 2018-2019 Peter Pearson.

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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "sniffle.h"

#define RUN_TESTS 0

#if RUN_TESTS
#include "tests/tests.h"
#endif

static void printHelp(bool fullOptions)
{
	fprintf(stderr, "Sniffle version 0.6.3. Copyright 2018-2022 Peter Pearson.\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "sniffle [options] find <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] find <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] [filter] find <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] grep <stringToFind> <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] [filter] grep <stringToFind> <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] count <stringToFind>  <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] match <tokens|to|find> <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] match <tokens&to&find> <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] tsdelta <mins> <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] debug <args>...    print args received.\n");
	fprintf(stderr, "\nNote: in most shells, a path with wildcards in will likely have to be escaped/quoted to prevent auto-completed arguments being given to Sniffle.\n");
	
	if (fullOptions)
	{
		fprintf(stderr, "\nSimple Options:\n");
		fprintf(stderr, " -firstOnly\t\t\tMatch only the first item in each file.\n");
		fprintf(stderr, " -m <count>\t\t\tMatch count.\n");
		fprintf(stderr, " -ft <thread_count>\t\tNumber of threads to use to find files.\n");
//		fprintf(stderr, " -gt <thread_count>\t\t\tNumber of threads to use to grep/process files.\n");
		fprintf(stderr, " -n\t\t\t\tOutput line numbers alongside content.\n");
		fprintf(stderr, " -rd <limit>\t\t\tDirectory recursion depth limit.\n");
		fprintf(stderr, " -sc <string>\t\t\tShort Circuit string.\n");
		fprintf(stderr, " -C <line_count>\t\tContext lines to print either side of match.\n");
		fprintf(stderr, " -B <line_count>\t\tContext lines to print before match.\n");
		fprintf(stderr, " -A <line_count>\t\tContext lines to print after match.\n");
		
		fprintf(stderr, "\nFilters:\n");
		fprintf(stderr, " -filefilter-moddate (-ff-md) <[y/o][6][h/d/w]>\n");
		fprintf(stderr, " -filefilter-size (-ff-s) <[b/s][6][k/m/g]>\n");

		Config tempConfig;
		tempConfig.printFullOptions();
	}
}

int main(int argc, char** argv)
{
#if RUN_TESTS
	SniffleTests tests;
	if (tests.testFilenameMatchers())
	{
		fprintf(stderr, "Tests ran okay.\n");
	}
	else
	{
		fprintf(stderr, "Tests failed.\n");
	}
	return 0;
#endif
	
	if (argc <= 1)
	{
		printHelp(false);
		return -1;
	}

	if (strcmp(argv[1], "debug") == 0)
	{
		fprintf(stderr, "Args provided:\n");
		for (int i = 1; i < argc; i++)
		{
			fprintf(stderr, "%s\n", argv[i]);
		}
		return 0;
	}
	
	Sniffle sniffle;
	
	// the assumption here is that optional options (starting with '-') are always the first arguments,
	// followed by any filters, followed by the main command + args for that...
	
	int nextArg = 1;
	
	// process any optional options
	Config::ParseResult res = sniffle.parseArgs(argc, argv, 1, nextArg);

	if (res == Config::eParseHelpWanted)
	{
		printHelp(true);
		return 0;
	}
	else if (res == Config::eParseError)
	{
		fprintf(stderr, "Error parsing option command line arguments.\n");
		return -1;
	}

	// Note: returns true even if it didn't need to do anything. Returns
	//       false on parse error.
	int nextArgFilter = 1;
	// don't bother making use of nextArgTemp here, as there's a bit of a
	// conflict of parseArgs() and parsefilter() doing the same thing, which
	// needs to be sorted out properly at some point...
	// Note: start from arg 1 again...
	// TODO: this double-parsing is not a good idea - redo this...
	bool parsedFilter = sniffle.parseFilter(argc, argv, 1, nextArgFilter);
	if (!parsedFilter)
	{
		fprintf(stderr, "Error parsing filter command line arguments.\n");
		return -1;
	}

	nextArg = std::max(nextArgFilter, nextArg);
	
	int commandArgs = argc - nextArg;
	if (commandArgs <= 0)
	{
		// something's gone very wrong.
		fprintf(stderr, "Error handling command line parsing.\n");
		return -1;
	}

	std::string mainCommand = argv[nextArg];
	
	if (mainCommand == "find")
	{
		if (commandArgs < 2)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'find' command.\n");
			return -1;
		}
		
		std::string path = argv[nextArg + 1];
		
		sniffle.runFind(path);
	}
	else if (mainCommand == "grep")
	{
		if (commandArgs < 3)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'grep' command.\n");
			return -1;
		}
		
		std::string contentsPattern = argv[nextArg + 1];
		std::string filePattern = argv[nextArg + 2];
		
		sniffle.runGrep(filePattern, contentsPattern);
	}
	else if (mainCommand == "count")
	{
		if (commandArgs < 3)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'count' command.\n");
			return -1;
		}
		
		std::string contentsPattern = argv[nextArg + 1];
		std::string filePattern = argv[nextArg + 2];
		
		sniffle.runCount(filePattern, contentsPattern);
	}
	else if (mainCommand == "match")
	{
		if (commandArgs < 3)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'match' command.\n");
			return -1;
		}
		
		std::string contentsPattern = argv[nextArg + 1];
		std::string filePattern = argv[nextArg + 2];
		
		sniffle.runMatch(filePattern, contentsPattern);
	}
	else if (mainCommand == "tsdelta")
	{
		if (commandArgs < 3)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'tsdelta' command.\n");
			return -1;
		}

		std::string tsDeltaParams = argv[nextArg + 1];

		uint64_t tsDelta = 0; // in minutes by default

		// see if we have non-numeric char at the end
		if (!isdigit(tsDeltaParams[tsDeltaParams.size() - 1]))
		{
			char postFixChar = tsDeltaParams[tsDeltaParams.size() - 1];
			std::string valueString = tsDeltaParams.substr(0, tsDeltaParams.size() - 1);
			tsDelta = atoi(valueString.c_str());

			if (postFixChar == 'h')
			{
				tsDelta *= 60;
			}
			else if (postFixChar == 'd')
			{
				tsDelta *= 60 * 24;
			}
		}
		else
		{
			tsDelta = atoi(tsDeltaParams.c_str());
		}

		std::string filePattern = argv[nextArg + 2];

		sniffle.runTimestampDeltaFind(filePattern, tsDelta * 60);
	}

	return 0;
}

