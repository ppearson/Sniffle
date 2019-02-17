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

	// for use on unknown directory entries to intelligently reject any obviously unwanted files
	// before bothering to stat() them (which is expensive).
	// struct dirent->d_type returned from readdir() can (validly) be DT_UNKNOWN in some cases,
	// especially older XFS filesystem versions and NFS mounts.
	
	// The implementation of this function must return false if there is no extension
	// (so possibly not a file).
	virtual bool canSkipPotentialFile(const char* filename) const = 0;
};

// extension only matcher, any core filename
class FilenameMatcherExtension : public FilenameMatcher
{
public:
	FilenameMatcherExtension(const std::string& extension) :
		m_extension(extension)
	{

	}

	virtual bool doesMatch(const std::string& filename) const override;

	virtual bool canSkipPotentialFile(const char* filename) const override;

protected:
	std::string		m_extension;
};

// designed for a wildcard match of the main filename - i.e. "tes*", "*es*", "*est", "t*t"
class FilenameMatcherNameWildcard : public FilenameMatcher
{
public:
	FilenameMatcherNameWildcard(const std::string& matchString, const std::string& extension);

	enum MatchType
	{
		eMTItemLeft,			// "tes*"
		eMTItemInner,			// "*es*"
		eMTItemRight,			// "*est"
		eMTItemOuter,			// "t*t"
		eMTItemFullWildcard		// "*"
	};

	virtual bool doesMatch(const std::string& filename) const override;

	virtual bool canSkipPotentialFile(const char* filename) const override;

protected:
	MatchType		m_matchType;
	std::string		m_filenameMatchItemMain; // used for all type
	std::string		m_filenameMatchItemExtra; // used for eMTItemsOuter for the right-side item match

	std::string		m_extension;
};

// matches exact filename, optionally with extension
class FilenameMatcherExactFilename : public FilenameMatcher
{
public:
	FilenameMatcherExactFilename(const std::string& filenameMatch,
							 const std::string& extensionMatch) :
		m_filenameMatch(filenameMatch),
		m_extensionMatch(extensionMatch)
	{

	}

	virtual bool doesMatch(const std::string& filename) const override;

	virtual bool canSkipPotentialFile(const char* filename) const override;

protected:
	std::string		m_filenameMatch;
	std::string		m_extensionMatch;
};

#endif // FILENAME_MATCHERS_H
