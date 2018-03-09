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

#include "string_helpers.h"

#include <stdlib.h>

StringHelpers::StringHelpers()
{
	
}

void StringHelpers::split(const std::string& str, std::vector<std::string>& tokens, const std::string& sep)
{
	int lastPos = str.find_first_not_of(sep, 0);
	int pos = str.find_first_of(sep, lastPos);

	while (lastPos != -1 || pos != -1)
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(sep, pos);
		pos = str.find_first_of(sep, lastPos);
	}
}

void StringHelpers::toLower(std::string& str)
{
	unsigned int size = str.size();
	for (unsigned int i = 0; i < size; i++)
		str[i] = tolower(str[i]);
}
