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

#ifndef FILE_GREPPER_H
#define FILE_GREPPER_H

#include <string>
#include <fstream>
#include <vector>

#include "utils/string_buffer.h"

class Config;

class FileGrepper
{
public:
	FileGrepper(const Config& config);
	~FileGrepper();

	// compute cached values for matching mode
	bool initMatch(const std::string& matchString);

	
	
	// operations per file...
	// TODO: abstract this to classes doing this	
	
	// basic ones are slow, naive versions that read files per-line using linear search
	
	// slow, basic
	bool grepBasic(const std::string& filename, const std::string& searchString, bool foundPreviousFile);
	
	bool countBasic(const std::string& filename, const std::string& searchString);

	// match - initMatch() has to have been called previously for this to work...
	bool matchBasic(const std::string& filename, bool foundPreviousFile);
	bool matchBasicOr(const std::string& filename, bool foundPreviousFile);
	bool matchBasicAnd(const std::string& filename, bool foundPreviousFile);

	bool findTimestampDelta(const std::string& filename, uint64_t timeDeltaSeconds, bool foundPreviousFile);


private:
	bool openFileStream(const std::string& filename, std::fstream& fileStream);
	
private:
	const Config&	m_config;
	
	enum MatchType
	{
		eMatchTypeOr,
		eMatchTypeAnd
	};

	// cached stuff
	unsigned int		m_readBufferSize;
	char*				m_pReadBuffer;
	
	bool				m_cacheBeforeLines;
	StringBuffer		m_stringBuffer;
	
	// match items
	MatchType			m_matchType;
	std::vector<std::string>	m_aMatchItems;
	
	bool				m_shortCircuit;
	std::string			m_shortCircuitString;
	
	bool				m_logTimestampSurround;
	char				m_logTimestampBeforeChar;
	char				m_logTimestampAfterChar;
	unsigned int		m_logTimestampMinLineLength;
};

#endif // FILE_GREPPER_H
