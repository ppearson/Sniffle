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

#include "sniffle.h"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "file_helpers.h"
#include "string_helpers.h"

#include "file_grepper.h"

#include "pattern_matchers.h"

Sniffle::Sniffle() : 
	m_pFilenameMatcher(nullptr)
{
	// load any local config file if one exists
	m_config.loadConfigFile();
}

Sniffle::~Sniffle()
{
	if (m_pFilenameMatcher)
	{
		delete m_pFilenameMatcher;
		m_pFilenameMatcher = nullptr;
	}
}

bool Sniffle::parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex)
{
	return m_config.parseArgs(argc, argv, startOptionArg, nextArgIndex);
}

void Sniffle::runFind(const std::string& pattern)
{
	std::vector<std::string> foundFiles;

	if (!findFiles(pattern, foundFiles, 0))
	{
		fprintf(stdout, "No files found.\n");
		return;
	}

	for (const std::string& fileItem : foundFiles)
	{
		fprintf(stdout, "%s\n", fileItem.c_str());
	}
}

void Sniffle::runGrep(const std::string& filePattern, const std::string& contentsPattern)
{
	// this can be done as a file find, plus additional contents search on the results.
	// TODO: contents search in multiple threads...

	std::vector<std::string> foundFiles;

	if (!findFiles(filePattern, foundFiles, 0))
	{
		fprintf(stderr, "No files found.\n");
		return;
	}

	FileGrepper grepper(m_config);

	bool foundPrevious = false;

	for (const std::string& fileItem : foundFiles)
	{
		// the grepper itself does any printing...
		foundPrevious |= grepper.findBasic(fileItem, contentsPattern, foundPrevious);
	}
}

//

Sniffle::PatternSearch Sniffle::classifyPattern(const std::string& pattern)
{
	Sniffle::PatternSearch result;

	result.baseSearchPath = "/";

	std::vector<std::string> patternTokens;
	StringHelpers::split(pattern, patternTokens, "/");

	// currently we only support one wildcard directory

	bool foundDirWildcard = false;

	// see if we've got a wildcard for a directory
	for (unsigned int i = 0; i < patternTokens.size(); i++)
	{
		const std::string& token = patternTokens[i];

		bool lastToken = i == (patternTokens.size() - 1);

		if (!lastToken && !foundDirWildcard && token.find("*") != std::string::npos && token.find(".") == std::string::npos)
		{
			// we've found a directory wildcard (that isn't a file wildcard)

			result.dirWildcardMatch = token;

			foundDirWildcard = true;
		}
//		else if (token.find(".") != std::string::npos && lastToken)
		else if (lastToken)
		{
			// it should be the file filer

			result.fileMatch = token;
		}
		else if (!lastToken)
		{
			// it should be just a string token

			if (!foundDirWildcard)
			{
				// if we haven't found wildcard dir token yet, add it to the base path
				result.baseSearchPath = FileHelpers::combinePaths(result.baseSearchPath, token);
			}
			else
			{
				// otherwise, add it to the remainders path to be looked for after the directory wildcard
				result.dirRemainders.push_back(token);
			}
		}
	}

	if (!result.fileMatch.empty())
	{
		if (foundDirWildcard)
		{
			result.type = ePatternWildcardDir;
		}
		else
		{
			result.type = ePatternSimple;
		}
	}

//	fprintf(stderr, "base path: %s\n", result.baseSearchPath.c_str());
//	fprintf(stderr, "file match: %s\n", result.fileMatch.c_str());

	return result;
}

bool Sniffle::configureFilenameMatcher(const PatternSearch& pattern)
{
	if (m_pFilenameMatcher)
	{
		delete m_pFilenameMatcher;
		m_pFilenameMatcher = nullptr;
	}
	
	// work out the type of file matcher we want.
	
	if (pattern.fileMatch == "*")
	{
		// TODO: could do a specialised matcher for this.
		m_pFilenameMatcher = new SimpleFilenameMatcher("*");
		return true;
	}
	
	size_t sepPos = pattern.fileMatch.find(".");
	if (sepPos == std::string::npos)
	{
		m_pFilenameMatcher = new SimpleFilenameMatcher("*");
		return true;
	}
	
	std::string filenameCorePart = pattern.fileMatch.substr(0, sepPos);
	std::string extensionPart = pattern.fileMatch.substr(sepPos + 1);
	
	// simple extension only filter
	if (extensionPart.find("*", sepPos) == std::string::npos && filenameCorePart == "*")
	{
		// just a simple filename extension filter
		m_pFilenameMatcher = new SimpleFilenameMatcher(extensionPart);
		return true;
	}
	
	// currently we only support partial wildcards in the filename part (not extension)
	// TODO: implement this correctly
	
	// otherwise, it's the more complex type
	if (filenameCorePart.find("*") == std::string::npos)
	{
		// something weird's going on, and we don't support this currently.
		fprintf(stderr, "Error creating filename matcher with file match string: %s\n", pattern.fileMatch.c_str());
		return false;
	}
	
	// for the moment, assume there are wildcards either side by replacing wildcard chars
	if (filenameCorePart[0] != '*' || filenameCorePart[filenameCorePart.size() - 1] != '*')
	{
		// something weird's going on, and we don't support this currently.
		fprintf(stderr, "Error creating filename matcher with file match string: %s\n", pattern.fileMatch.c_str());
		return false;
	}
	
	std::string filenameMatchItem = filenameCorePart.substr(1, filenameCorePart.size() - 2);
	
	m_pFilenameMatcher = new AdvancedFilenameMatcher(filenameMatchItem, extensionPart);
	return true;
}

bool Sniffle::findFiles(const std::string& pattern, std::vector<std::string>& foundFiles, unsigned int findFlags)
{
	// currently we assume we're given an absolute path, but...

	std::vector<std::string> patternTokens;
	StringHelpers::split(pattern, patternTokens, "/");

	PatternSearch patternRes = classifyPattern(pattern);
	
	if (!configureFilenameMatcher(patternRes))
	{
		fprintf(stderr, "Couldn't understand search terms.\n");
		return false;
	}

	if (patternRes.type == ePatternSimple)
	{
		// just do a simple recursive search

		// currently we only support extension wildcards...
		std::string extensionMatch = "*";
		if (patternRes.fileMatch.find(".") != std::string::npos)
		{
			extensionMatch = patternRes.fileMatch.substr(patternRes.fileMatch.find(".") + 1);
		}

		bool foundOK = getRelativeFilesInDirectoryRecursive(patternRes.baseSearchPath, "", foundFiles);

		if (foundOK)
		{
			// the results of the above are relative, so add the base search path

			for (std::string& item : foundFiles)
			{
				item = FileHelpers::combinePaths(patternRes.baseSearchPath, item);
			}
		}

		return foundOK;
	}
	else if (patternRes.type == ePatternWildcardDir)
	{
		// for this type of search, we look for directories matching the wildcard (currently just *) as a first step

		std::vector<std::string> wildCardDirs;
		if (!FileHelpers::getDirectoriesInDirectory(patternRes.baseSearchPath, patternRes.dirWildcardMatch, wildCardDirs))
		{
			return false;
		}

		// currently we only support extension wildcards...
		std::string extensionMatch = "*";
		if (patternRes.fileMatch.find(".") != std::string::npos)
		{
			extensionMatch = patternRes.fileMatch.substr(patternRes.fileMatch.find(".") + 1);
		}

		bool foundSomeFiles = false;

		// we have some first level directories where the wildcard is, so for each of those try and find remainder directories within each
		for (const std::string& wildcardDir : wildCardDirs)
		{
			std::string testDir = FileHelpers::combinePaths(patternRes.baseSearchPath, wildcardDir);

			// TODO: this check technically isn't needed, but for the moment it's helpful to guarentee things are doing what they should be,
			//       and there might be issues with symlinks...
			DIR* dir = opendir(testDir.c_str());
			if (!dir)
			{
				fprintf(stderr, "Error: 55\n");
				continue;
			}
			closedir(dir);

			bool dirToCheck = true;

			std::string remainderFullDir = testDir;

			for (const std::string& remainderDir : patternRes.dirRemainders)
			{
				testDir = FileHelpers::combinePaths(testDir, remainderDir);

				dir = opendir(testDir.c_str());

				if (!dir)
				{
					dirToCheck = false;
					break;
				}
				closedir(dir);

				remainderFullDir = FileHelpers::combinePaths(remainderFullDir, remainderDir);
			}

			if (!dirToCheck)
				continue;

			// otherwise, we should have a final directory matching the pattern, including the directory wildcard.
			// so now do a file search at that level

			// TODO: could parallelise this, but we need to be a bit careful as we really care about latency at this point, so
			//       spawing off threads to do further file globbing needs care... It *might* make some sense over NFS though...

			bool foundOK = getRelativeFilesInDirectoryRecursive(remainderFullDir, remainderFullDir, foundFiles);
			foundSomeFiles |= foundOK;
		}

		return foundSomeFiles;
	}

	return false;
}

bool Sniffle::getRelativeFilesInDirectoryRecursive(const std::string& searchDirectoryPath, const std::string& relativeDirectoryPath,
													 std::vector<std::string>& files) const
{
	DIR* dir = opendir(searchDirectoryPath.c_str());
	if (!dir)
		return false;
	
	struct dirent* dirEnt = NULL;
	char tempBuffer[4096];

	while ((dirEnt = readdir(dir)) != NULL)
	{
		if (dirEnt->d_type == DT_DIR)
		{
			// ignore built-in items
			if (strcmp(dirEnt->d_name, ".") == 0 || strcmp(dirEnt->d_name, "..") == 0)
				continue;

			// build up next directory level relative path
			std::string newFullDirPath = FileHelpers::combinePaths(searchDirectoryPath, dirEnt->d_name);
			std::string newRelativeDirPath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
			getRelativeFilesInDirectoryRecursive(newFullDirPath, newRelativeDirPath, files);
		}
		else if (dirEnt->d_type == DT_LNK)
		{
			// cope with symlinks by working out what they point at
			std::string fullAbsolutePath = FileHelpers::combinePaths(searchDirectoryPath, dirEnt->d_name);
			ssize_t linkTargetStringSize = readlink(fullAbsolutePath.c_str(), tempBuffer, 4096);
			if (linkTargetStringSize == -1)
			{
				// something went wrong, so ignore...
				continue;
			}
			else
			{
				// readlink() doesn't put a null-terminator on the string, so we have to do that...
				tempBuffer[linkTargetStringSize] = 0;
				// on the assumption that the target of the symlink is not another symlink (if so, this won't work over NFS)
				// check what type it is
				struct stat statState;
				int ret = lstat(tempBuffer, &statState);

				if (ret == -1)
				{
					// ignore for the moment...
					continue;
				}
				else if (S_ISDIR(statState.st_mode))
				{
					// build up next directory level relative path
					std::string newFullDirPath = tempBuffer;
					std::string newRelativeDirPath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
					getRelativeFilesInDirectoryRecursive(newFullDirPath, newRelativeDirPath, files);
				}
				else
				{
					// it's a file
				}
			}
		}
		else
		{
			// it's hopefully a file
			
			// see if it's what we want...
			
			if (m_pFilenameMatcher->doesMatch(dirEnt->d_name))
			{
				std::string fullRelativePath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
				files.push_back(fullRelativePath);
			}
		}
	}

	closedir(dir);

	return !files.empty();
}
