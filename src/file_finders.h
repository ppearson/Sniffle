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

#ifndef FILE_FINDERS_H
#define FILE_FINDERS_H

#include <vector>
#include <string>

class Config;
class FilenameMatcher;
struct PatternSearch;

class FileFinder
{
public:
	FileFinder(const Config& config,
	           const FilenameMatcher* pFilenameMatcher,
	           const PatternSearch& patternSearch);
	
	virtual ~FileFinder();
	
	bool getRelativeFilesInDirectoryRecursive(const std::string& searchDirectoryPath, const std::string& relativeDirectoryPath,
											  unsigned int currentDepth, std::vector<std::string>& files) const;
	
	virtual bool findFiles(std::vector<std::string>& foundFiles) const = 0;	
	
protected:
	const Config&			m_config;
	const FilenameMatcher*	m_pFilenameMatcher;
	const PatternSearch&	m_patternSearch;
};

//

class FileFinderBasicRecursive : public FileFinder
{
public:
	FileFinderBasicRecursive(const Config& config,
							 const FilenameMatcher* pFilenameMatcher,
							 const PatternSearch& patternSearch);
	
	virtual bool findFiles(std::vector<std::string>& foundFiles) const;
};

//

class FileFinderBasicRecursiveDirectoryWildcard : public FileFinder
{
public:
	FileFinderBasicRecursiveDirectoryWildcard(const Config& config,
											  const FilenameMatcher* pFilenameMatcher,
											  const PatternSearch& patternSearch);
	
	virtual bool findFiles(std::vector<std::string>& foundFiles) const;
};

#endif // FILE_FINDERS_H
