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

#include <stdio.h>

#include <string>

#include "sniffle.h"

static void printHelp(bool fullOptions)
{
	fprintf(stderr, "Sniffle version 0.1.\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "sniffle [options] find <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] find <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] grep <stringToFind> <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] grep <stringToFind> <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] match <tokens|to|find> <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] match <tokens&to&find> <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle [options] debug <args>...    print args received.\n");
	fprintf(stderr, "\nNote: in some shells, a path with wildcards in might have to be escaped/quoted to prevent auto-completed arguments being given to Sniffle.\n");
	
	if (fullOptions)
	{
		fprintf(stderr, "\nSimple Options:\n");
		fprintf(stderr, " -firstOnly\t\t\tMatch only the first item in each file.\n");
		fprintf(stderr, " -m <count>\t\t\tMatch count.\n");
		fprintf(stderr, " -ft <thread_count>\t\tNumber of threads to use to find files.\n");
//		fprintf(stderr, " -gt <thread_count>\t\t\tNumber of threads to use to grep/process files.\n");
		fprintf(stderr, " -n\t\t\t\tOutput line numbers alongside content.\n");
		fprintf(stderr, " -rd <limit>\t\t\tDirectory recursion depth limit.\n");
		fprintf(stderr, " -C <line_count>\t\tContext lines to print either side of match.\n");
//		fprintf(stderr, " -B <line_count>\t\tContext lines to print before match.\n");
		fprintf(stderr, " -A <line_count>\t\tContext lines to print after match.\n");

		Config tempConfig;
		tempConfig.printFullOptions();
	}
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		printHelp(false);
		return -1;
	}
	
	Sniffle sniffle;
	
	// the assumption here is that optional options (starting with '-') are always the first arguments...
	
	int nextArg = 1;
	
	// process any optional options
	Config::ParseResult res = sniffle.parseArgs(argc, argv, 1, nextArg);
	
	int commandArgs = argc - nextArg;
	
	std::string mainCommand = argv[nextArg];
	
	if (mainCommand == "find")
	{
		if (commandArgs < 2)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'find' command.\n");
			return 0;
		}
		
		std::string path = argv[nextArg + 1];
		
		sniffle.runFind(path);
	}
	else if (mainCommand == "grep")
	{
		if (commandArgs < 3)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'grep' command.\n");
			return 0;
		}
		
		std::string contentsPattern = argv[nextArg + 1];
		std::string filePattern = argv[nextArg + 2];
		
		sniffle.runGrep(filePattern, contentsPattern);
	}
	else if (mainCommand == "match")
	{
		if (commandArgs < 3)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'match' command.\n");
			return 0;
		}
		
		std::string contentsPattern = argv[nextArg + 1];
		std::string filePattern = argv[nextArg + 2];
		
		sniffle.runMatch(filePattern, contentsPattern);
	}
	else if (mainCommand == "count")
	{
		
	}
	else if (mainCommand == "debug")
	{
		fprintf(stderr, "Args provided:\n");
		for (int i = 1; i < argc; i++)
		{
			fprintf(stderr, "%s\n", argv[i]);
		}
	}
	else if (res == Config::eParseHelpWanted)
	{
		printHelp(true);
		return -1;
	}
	
	return 0;
}

