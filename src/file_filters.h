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

#ifndef FILE_FILTERS_H
#define FILE_FILTERS_H

#include <time.h>

class FilterParameters
{
public:
	
	// TODO: this now currently supports one of each type, but not two of each type, so still not great...
	// TODO: maybe chaining together of filter ops?
	enum FilterFlags
	{
		FILTER_NONE							= 0,
		FILTER_FILEMODIFIEDDATE_YOUNGER		= 1 << 0,
		FILTER_FILEMODIFIEDDATE_OLDER		= 1 << 1,
		FILTER_FILESIZE_LARGER				= 1 << 2,
		FILTER_FILESIZE_SMALLER				= 1 << 3
	};
	
	FilterParameters() : m_filterTypeFlags(0)
	{
	}

	unsigned int getFilterTypeFlags() const
	{
		return m_filterTypeFlags;
	}

	time_t getModifiedTimestampThreshold() const
	{
		return m_modifiedTimestampThreshold;
	}

	size_t getFileSizeThreshold() const
	{
		return m_fileSizeThreshold;
	}
	
	// this isn't very principled, but...
	void setFileModifiedDateFilter(bool younger, unsigned int deltaThresholdInHours);

	void setFileSizeFilter(bool larger, size_t fileSizeThresholdInBytes);
	
protected:
	unsigned int		m_filterTypeFlags;

	time_t				m_modifiedTimestampThreshold;
	size_t				m_fileSizeThreshold;
};

#endif // FILE_FILTERS_H
