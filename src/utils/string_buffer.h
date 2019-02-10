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

#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include <vector>
#include <string>

// helper class for Basic grep methods to store line buffers

class StringBuffer
{
public:
	StringBuffer() :
		m_bufferSize(0),
		m_currentIndex(0),
		m_itemCount(0)
	{
	}

	void init(unsigned int numStrings, unsigned int stringLength)
	{
		m_buffers.resize(numStrings);

		m_bufferSize = numStrings;

		for (std::string& buffer : m_buffers)
		{
			buffer.resize(stringLength);
		}

		m_currentIndex = 0;
		m_itemCount = 0;
	}

	void reset()
	{
		m_currentIndex = 0;
		m_itemCount = 0;
	}

	char* getNextBuffer()
	{
		char* buffer = (char*)m_buffers[m_currentIndex].c_str();

		m_currentIndex = (m_currentIndex + 1) % m_bufferSize;

		m_itemCount++;

		return buffer;
	}

	// these should be positive indices back - starting at the max size counting down to 1
	const char* getPreviousBuffer(unsigned int index) const
	{
		// do this as signed...
		int offset = ((int)index - (int)m_currentIndex);

		unsigned int maxNum = std::min(m_bufferSize, m_itemCount);

		if (offset < 0)
		{
			offset += maxNum;
		}

		offset = maxNum - offset - 1;

//		fprintf(stderr, " lastIndex: %i\n", offset);

		return m_buffers[offset].c_str();
	}


protected:
	// TODO: allocate this in one big block to avoid cache misses?
	std::vector<std::string>	m_buffers;

	unsigned int				m_bufferSize;
	unsigned int				m_currentIndex;
	unsigned int				m_itemCount;
};

#endif // STRING_BUFFER_H
