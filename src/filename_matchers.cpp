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

#include "filename_matchers.h"

#include <string.h>

#include "utils/file_helpers.h"

// TODO: if CPU performance ever becomes an issue, remove some of these string copies...

bool FilenameMatcherExtension::doesMatch(const std::string& filename) const
{
	if (m_extension == "*" || FileHelpers::getFileExtension(filename) == m_extension)
	{
		return true;
	}

	return false;
}

bool FilenameMatcherExtension::canSkipPotentialSymlinkFile(const char* filename) const
{
	const char* dotPos = strrchr(filename, '.');

	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}

	// otherwise, see if the extension matches. The current assumption is that at least
	// the file extension will match between the symlink filename and the target of the symlink.

	if (m_extension == "*")
		return false;

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

	const std::string& mainFilename = FileHelpers::stripExtensionFromFilename(filename);

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

bool FilenameMatcherNameWildcard::canSkipPotentialSymlinkFile(const char* filename) const
{
	const char* dotPos = strrchr(filename, '.');

	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}

	// otherwise, see if the extension matches. The current assumption is that at least
	// the file extension will match between the symlink filename and the target of the symlink.

	if (m_extension == "*")
		return false;

	int compValue = strcmp(dotPos + 1, m_extension.c_str());
	return (compValue != 0);
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

bool FilenameMatcherExactFilename::canSkipPotentialSymlinkFile(const char* filename) const
{
	const char* dotPos = strrchr(filename, '.');

	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}

	// otherwise, see if the extension matches. The current assumption is that at least
	// the file extension will match between the symlink filename and the target of the symlink.

	if (m_extensionMatch == "*")
		return false;

	int compValue = strcmp(dotPos + 1, m_extensionMatch.c_str());
	return (compValue != 0);
}
