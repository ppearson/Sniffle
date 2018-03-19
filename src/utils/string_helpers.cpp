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
#include <algorithm>

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

std::string StringHelpers::formatSize(size_t amount)
{
	char szMemAvailable[16];
	std::string units;
	unsigned int size = 0;
	char szDecimalSize[12];
	if (amount >= 1024 * 1024 * 1024) // GB
	{
		size = amount / (1024 * 1024);
		float fSize = (float)size / 1024.0f;
		sprintf(szDecimalSize, "%.2f", fSize);
		units = "GB";
	}
	else if (amount >= 1024 * 1024) // MB
	{
		size = amount / 1024;
		float fSize = (float)size / 1024.0f;
		sprintf(szDecimalSize, "%.2f", fSize);
		units = "MB";
	}
	else if (amount >= 1024) // KB
	{
		size = amount;
		float fSize = (float)size / 1024.0f;
		sprintf(szDecimalSize, "%.1f", fSize);
		units = "KB";
	}
	else
	{
		sprintf(szDecimalSize, "0");
		units = "B"; // just so it makes sense
	}

	sprintf(szMemAvailable, "%s %s", szDecimalSize, units.c_str());
	std::string final(szMemAvailable);
	return final;
}

std::string StringHelpers::formatNumberThousandsSeparator(size_t value)
{
	char szRawNumber[32];
	sprintf(szRawNumber, "%zu", value);

	std::string temp(szRawNumber);

	std::string final;
	int i = temp.size() - 1;
	unsigned int count = 0;
	for (; i >= 0; i--)
	{
		final += temp[i];

		if (count++ == 2 && i != 0)
		{
			final += ",";
			count = 0;
		}
	}

	std::reverse(final.begin(), final.end());

	return final;
}
