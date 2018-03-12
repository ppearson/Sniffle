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

#ifndef SNIFFLE_H
#define SNIFFLE_H

#include <string>
#include <vector>

#include "config.h"

class Sniffle
{
public:
	Sniffle();
	~Sniffle();
	
	bool parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex);
		
	void runFind(const std::string& pattern);
	
	void runGrep(const std::string& filePattern, const std::string& contentsPattern);
	
private:
	
	enum PatternType
	{
		ePatternUnknown,
		ePatternSimple,
		ePatternWildcardDir
	};
	
	struct PatternSearch
	{
		PatternSearch() : type(ePatternUnknown)
		{
		}
			
		
		PatternType			type;
		std::string			baseSearchPath;
		
		std::string			dirWildcardMatch;
		std::vector<std::string> dirRemainders;
		
		std::string			fileMatch;
	};
	
	
	
	enum FindFlags
	{
		FIND_RECURSIVE				= 1 << 0,
		FIND_FOLLOW_SYMLINKS		= 1 << 1,
		FIND_OUTPUT_RELATIVE_PATHS	= 1 << 2
	};
	
	static PatternSearch classifyPattern(const std::string& pattern);
	
	bool findFiles(const std::string& pattern, std::vector<std::string>& foundFiles, unsigned int findFlags);
	
	bool findFilesInDir(const std::string& pattern, std::vector<std::string>& foundFiles, unsigned int findFlags);
	
	
private:
	Config		m_config;
	
	
};

#endif // SNIFFLE_H