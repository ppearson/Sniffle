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

#include "file_filters.h"

const unsigned int kSecondsInDay = 3600;

// for setting as a delta from current time
void FilterParameters::setFileModifiedDateFilter(bool younger, unsigned int deltaThresholdInHours)
{
	m_filterType = (younger) ? eFilterFileModifiedDate_Younger : eFilterFileModifiedDate_Older;
	
	// get current time.
	// TODO: think about time-zones?
	time(&m_modifiedTimestampThreshold);
	
	time_t finalThresholdDelta = deltaThresholdInHours * kSecondsInDay;

	// now subtract this value from the current time value
	m_modifiedTimestampThreshold -= finalThresholdDelta;
}

