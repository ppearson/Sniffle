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

#include "sniffle.h"

#include <stdio.h>

#include "file_helpers.h"
#include "string_helpers.h"

Sniffle::Sniffle()
{
	
}

Sniffle::~Sniffle()
{
	
}

void Sniffle::runFind(const std::string& pattern)
{
	std::vector<std::string> foundFiles;
	findFiles(pattern, foundFiles, 0);
	
	
}

//

Sniffle::PatternSearch Sniffle::classifyPattern(const std::string& pattern)
{
	Sniffle::PatternSearch result;
	
	result.baseSearchPath = "/";
	
	std::vector<std::string> patternTokens;
	StringHelpers::split(pattern, patternTokens, "/");
	
	// currently we only support one wildcard directory
	
	bool foundDirWildcard = false;
	
	// see if we've got a wildcard for a directory
	for (unsigned int i = 0; i < patternTokens.size(); i++)
	{
		const std::string& token = patternTokens[i];
		
		if (!foundDirWildcard && token.find("*") != std::string::npos && token.find(".") == std::string::npos)
		{
			// we've found a directory wildcard (that isn't a file wildcard)
			
			
			
			foundDirWildcard = true;
		}
		else if (token.find(".") != std::string::npos)
		{
			
		}
		else
		{
			// it should be just a string token
			
			if (!foundDirWildcard)
			{
				// if we haven't found wildcard dir token yet, add it to the base path
				result.baseSearchPath = FileHelpers::combinePaths(result.baseSearchPath, token);
			}
			else
			{
				// otherwise, add it to the remainders path to be looked for after the directory wildcard
				result.dirRemainders.push_back(token);
			}
		}
			
		
		
	}
	
	fprintf(stderr, "base path: %s\n", result.baseSearchPath.c_str());
	
	return result;
}

bool Sniffle::findFiles(const std::string& pattern, std::vector<std::string>& foundFiles, unsigned int findFlags)
{
	// currently we assume we're given an absolute path, but...
	
	std::vector<std::string> patternTokens;
	StringHelpers::split(pattern, patternTokens, "/");
	
	classifyPattern(pattern);	
	
	return true;
}
