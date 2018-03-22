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

bool SimpleFilenameMatcher::doesMatch(const std::string& filename) const
{
	if (m_extension == "*" || FileHelpers::getFileExtension(filename) == m_extension)
	{
		return true;
	}

	return false;
}

bool SimpleFilenameMatcher::canSkipPotentialSymlinkFile(const char* filename) const
{
	const char* dotPos = strrchr(filename, '.');

	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}

	// otherwise, see if the extension matches. The current assumption is that at least
	// the file extension will match between the symlink filename and the target of the symlink.

	int compValue = strcmp(dotPos + 1, m_extension.c_str());
	return (compValue != 0);
}

/////

bool AdvancedFilenameMatcher::doesMatch(const std::string& filename) const
{
	if ((m_extension == "*" || FileHelpers::getFileExtension(filename) == m_extension) &&
			filename.find(m_filenameItem) != std::string::npos)
	{
		return true;
	}

	return false;
}

bool AdvancedFilenameMatcher::canSkipPotentialSymlinkFile(const char* filename) const
{
	const char* dotPos = strrchr(filename, '.');

	if (!dotPos)
	{
		// it's very unlikely to be a filename, so we can't skip it...
		return false;
	}

	// otherwise, see if the extension matches. The current assumption is that at least
	// the file extension will match between the symlink filename and the target of the symlink.

	int compValue = strcmp(dotPos + 1, m_extension.c_str());
	return (compValue != 0);
}
