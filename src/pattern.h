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

#ifndef PATTERN_H
#define PATTERN_H

#include <string>
#include <vector>

struct PatternSearch
{
	enum PatternType
	{
		ePatternUnknown,
		ePatternSingleFile,
		ePatternSimple,
		ePatternWildcardDir,
		ePatternError
	};
	
	PatternSearch() : type(ePatternUnknown)
	{
	}


	PatternType			type;
	std::string			baseSearchPath;

	std::string			dirWildcardMatch;
	std::vector<std::string> dirRemainders;

	std::string			fileMatch;
};

#endif // PATTERN_H

