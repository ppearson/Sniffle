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

#ifndef FILENAME_MATCHERS_H
#define FILENAME_MATCHERS_H

#include <string>

class FilenameMatcher
{
public:
	FilenameMatcher()
	{
	}

	virtual ~FilenameMatcher()
	{
	}

	// potential full match of filename and extension (it's up to what derived classes do though,
	// they might only look at the extension)
	virtual bool doesMatch(const std::string& filename) const = 0;

	// for use on symlinks to intelligently reject any obviously unwanted files before bothering
	// to stat() them. Must return false if there is no extension (so possibly not a file)
	virtual bool canSkipPotentialSymlinkFile(const char* filename) const = 0;
};

// extension only matcher
class SimpleFilenameMatcher : public FilenameMatcher
{
public:
	SimpleFilenameMatcher(const std::string& extension) :
		m_extension(extension)
	{

	}

	virtual bool doesMatch(const std::string& filename) const;

	virtual bool canSkipPotentialSymlinkFile(const char* filename) const;

protected:
	std::string		m_extension;
};

// more advanced version supporting primitive filename wildcards
class AdvancedFilenameMatcher : public FilenameMatcher
{
public:
	AdvancedFilenameMatcher(const std::string& filenameItem, const std::string& extension) :
		m_filenameItem(filenameItem),
		m_extension(extension)
	{

	}

	virtual bool doesMatch(const std::string& filename) const;

	virtual bool canSkipPotentialSymlinkFile(const char* filename) const;

protected:
	std::string		m_filenameItem;
	std::string		m_extension;
};

#endif // FILENAME_MATCHERS_H
