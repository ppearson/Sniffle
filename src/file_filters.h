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
	
	// currently we only support one active filter type
	enum FilterType
	{
		eFilterNone,
		eFilterFileModifiedDate_Younger,
		eFilterFileModifiedDate_Older,
		eFilterFileSize_Greater,
		eFilterFileSize_Smaller
	};
	
	FilterParameters() : m_filterType(eFilterNone)
	{
	}

	FilterType getFilterType() const
	{
		return m_filterType;
	}

	time_t getModifiedTimestampThreshold() const
	{
		return m_modifiedTimestampThreshold;
	}
	
	// this isn't very principled, but...
	void setFileModifiedDateFilter(bool younger, unsigned int deltaThresholdInHours);
	
protected:
	FilterType			m_filterType;

	time_t				m_modifiedTimestampThreshold;
	
};

#endif // FILE_FILTERS_H
