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

#include "filename_matchers.h"

#include <string.h>

#include "utils/file_helpers.h"
#include "utils/string_helpers.h"

// TODO: if CPU performance ever becomes an issue, remove some of these string copies...

bool FilenameMatcherExtension::doesMatch(const std::string& filename) const
{
	if (m_extension == "*" || FileHelpers::getFileExtension(filename) == m_extension)
	{
		return true;
	}

	return false;
}

bool FilenameMatcherExtension::canSkipPotentialFile(const char* filename) const
{
	const char* dotPos = strrchr(filename, '.');
	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}
	
	if (m_extension == "*")
		return false;

	// otherwise, see if the extension matches.
	int compValue = strcmp(dotPos + 1, m_extension.c_str());
	return (compValue != 0);
}

/////


FilenameMatcherNameWildcard::FilenameMatcherNameWildcard(const std::string& matchString, const std::string& extension) :
	   m_extension(extension)
{
	if (matchString == "*")
	{
		m_matchType = eMTItemFullWildcard;
	}
	else
	{
		if (matchString[0] == '*' && matchString[matchString.size() - 1] == '*')
		{
			m_matchType = eMTItemInner;
			m_filenameMatchItemMain = matchString.substr(1, matchString.size() - 2);
		}
		else if (matchString[matchString.size() - 1] == '*')
		{
			m_matchType = eMTItemLeft;
			m_filenameMatchItemMain = matchString.substr(0, matchString.size() - 1);
		}
		else if (matchString[0] == '*')
		{
			m_matchType = eMTItemRight;
			m_filenameMatchItemMain = matchString.substr(1, matchString.size() - 1);
		}
		else
		{
			m_matchType = eMTItemOuter;
			size_t astPos = matchString.find("*");
			if (astPos != std::string::npos)
			{
				m_filenameMatchItemMain = matchString.substr(0, astPos);
				m_filenameMatchItemExtra = matchString.substr(astPos + 1);
			}
		}
	}
}

bool FilenameMatcherNameWildcard::doesMatch(const std::string& filename) const
{
	if (m_extension != "*" && FileHelpers::getFileExtension(filename) != m_extension)
		return false;

	if (m_matchType == eMTItemFullWildcard)
		return true;

	const std::string mainFilename = FileHelpers::stripExtensionFromFilename(filename);

	// TODO: remove substr() allocations below by doing direct comparisons...

	if (m_matchType == eMTItemInner)
	{
		return mainFilename.find(m_filenameMatchItemMain) != std::string::npos;
	}
	else if (m_matchType == eMTItemLeft)
	{
		return mainFilename.substr(0, m_filenameMatchItemMain.size()) == m_filenameMatchItemMain;
	}
	else if (m_matchType == eMTItemRight)
	{
		// can't match...
		if (mainFilename.size() < m_filenameMatchItemMain.size())
		{
			return false;
		}

		size_t diff = mainFilename.size() - m_filenameMatchItemMain.size();
		return mainFilename.substr(diff, mainFilename.size()) == m_filenameMatchItemMain;
	}
	else if (m_matchType == eMTItemOuter)
	{
		// can't match...
		if (mainFilename.size() < m_filenameMatchItemMain.size() + m_filenameMatchItemExtra.size())
		{
			return false;
		}

		bool leftMatches = mainFilename.substr(0, m_filenameMatchItemMain.size()) == m_filenameMatchItemMain;
		if (!leftMatches)
			return false;

		size_t rightStartPos = mainFilename.size() - m_filenameMatchItemExtra.size();
		bool rightMatches = mainFilename.substr(rightStartPos) == m_filenameMatchItemExtra;
		return rightMatches;
	}

	return false;
}

bool FilenameMatcherNameWildcard::canSkipPotentialFile(const char* filename) const
{
	if (m_matchType == eMTItemFullWildcard)
		return false;
	
	const char* dotPos = strrchr(filename, '.');
	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}
	
	if (m_extension != "*")
	{
		// otherwise, see if the extension matches.
		bool extensionMatches = strcmp(dotPos + 1, m_extension.c_str()) == 0;
		// if it doesn't, we can skip it
		if (!extensionMatches)
			return true;
	}
	
	// if it does, do a more thorough check on the core filename. We *could* (maybe should?) use
	// doesMatch(), but that involves a further string alloc and copy, and long-term it'd be
	// nice to reduce those to as few as possible, so we'll implement a different version for the moment...
	
	size_t mainFilenameLength = dotPos - filename;
	
	if (m_matchType == eMTItemInner)
	{
		// TODO: this isn't completely correct, as we're also potentially matching in the extension of "filename"...
		const char* findItem = strstr(filename, m_filenameMatchItemMain.c_str());
		// we didn't find the string, so we can skip it.
		if (findItem == nullptr)
			return true;
	}
	else if (m_matchType == eMTItemLeft)
	{
		bool leftMatches = StringHelpers::stringMatches(m_filenameMatchItemMain.c_str(), m_filenameMatchItemMain.size(),
														filename, 0);
		return !leftMatches;
	}
	else if (m_matchType == eMTItemRight)
	{
		// can't match...
		if (mainFilenameLength < m_filenameMatchItemMain.size())
		{
			// skip it
			return true;
		}

		size_t diff = mainFilenameLength - m_filenameMatchItemMain.size();
		bool rightMatches = StringHelpers::stringMatches(m_filenameMatchItemMain.c_str(), m_filenameMatchItemMain.size(),
														 filename, diff);
		return !rightMatches;
	}
	else if (m_matchType == eMTItemOuter)
	{
		if (mainFilenameLength < m_filenameMatchItemMain.size() + m_filenameMatchItemExtra.size())
		{
			// skip it
			return true;
		}
		
		bool leftMatches = StringHelpers::stringMatches(m_filenameMatchItemMain.c_str(), m_filenameMatchItemMain.size(),
														filename, 0);
		if (!leftMatches)
		{
			return true;
		}
		
		size_t diff = mainFilenameLength - m_filenameMatchItemExtra.size();
		bool rightMatches = StringHelpers::stringMatches(m_filenameMatchItemExtra.c_str(), m_filenameMatchItemExtra.size(),
														 filename, diff);
		
		if (!rightMatches)
		{
			return true;
		}
	}
	
	return false;
}

////

bool FilenameMatcherExactFilename::doesMatch(const std::string& filename) const
{
	size_t sepPos = filename.find(".");
	if (sepPos == std::string::npos)
	{
		return (m_extensionMatch.empty() || m_extensionMatch == "*") && m_filenameMatch == filename;
	}

	std::string filenameCorePart = filename.substr(0, sepPos);
	std::string extensionPart = filename.substr(sepPos + 1);

	if (m_extensionMatch != "*" && extensionPart != m_extensionMatch)
	{
		return false;
	}

	return m_filenameMatch == filenameCorePart;
}

bool FilenameMatcherExactFilename::canSkipPotentialFile(const char* filename) const
{
	const char* dotPos = strrchr(filename, '.');
	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}

	// otherwise, see if the extension matches.

	if (m_extensionMatch == "*")
		return false;

	int compValue = strcmp(dotPos + 1, m_extensionMatch.c_str());
	return (compValue != 0);
}
