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

static void printHelp()
{
	fprintf(stderr, "Sniffle version 0.1.\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "sniffle find [options] <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle find [options] <\"/path/to/*/search/*.log\">\n");
	fprintf(stderr, "sniffle grep [options] <stringToFind> <\"/path/to/search/*.log\">\n");
	fprintf(stderr, "sniffle grep [options] <stringToFind> <\"/path/to/*/search/*.log\">\n");
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		printHelp();
		return -1;
	}
	
	Sniffle sniffle;

	std::string mainCommand = argv[1];
	
	
	int nextArg = 2;
	sniffle.parseArgs(argc, argv, 2, nextArg);
	
	if (mainCommand == "find")
	{
		if (argc < 3)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'find' command.\n");
			return 0;
		}
		
		std::string path = argv[2];
		
		sniffle.runFind(path);
	}
	else if (mainCommand == "grep")
	{
		if (argc < 4)
		{
			fprintf(stderr, "Error: Insufficient number of arguments for 'grep' command.\n");
			return 0;
		}
		
		std::string contentsPattern = argv[2];
		std::string filePattern = argv[3];
		
		sniffle.runGrep(filePattern, contentsPattern);
	}
	else if (mainCommand == "match")
	{
		
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
	else if (mainCommand.find("help") != std::string::npos)
	{
		printHelp();
		return -1;
	}
	
	return 0;
}

