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
#include <stdlib.h>
#include <locale.h>

#include "utils/file_helpers.h"
#include "utils/string_helpers.h"
#include "utils/system_helpers.h"

#include "file_grepper.h"

#include "filename_matchers.h"

Sniffle::Sniffle() :
	m_pFilenameMatcher(nullptr)
{
	// load any local config file if one exists
	m_config.loadConfigFile();

	configureGlobals();
}

Sniffle::~Sniffle()
{
	if (m_pFilenameMatcher)
	{
		delete m_pFilenameMatcher;
		m_pFilenameMatcher = nullptr;
	}
}

Config::ParseResult Sniffle::parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex)
{
	return m_config.parseArgs(argc, argv, startOptionArg, nextArgIndex);
}

bool Sniffle::configureGlobals()
{
	setlocale(LC_ALL, "C");
//	char const* previousLocale = setlocale(LC_ALL, "C");
//	fprintf(stderr, "Prev locale: %s\n", previousLocale);

	return true;
}

void Sniffle::runFind(const std::string& pattern)
{
	std::vector<std::string> foundFiles;

	fprintf(stderr, "Searching for files...\n");

	if (!findFiles(pattern, foundFiles, 0))
	{
		fprintf(stderr, "No files found matching file match criteria.\n");
		return;
	}

	for (const std::string& fileItem : foundFiles)
	{
		fprintf(stdout, "%s\n", fileItem.c_str());
	}

	if (SystemHelpers::isStdOutATTY())
	{
		fprintf(stderr, "\nFound %zu %s.\n", foundFiles.size(), foundFiles.size() == 1 ? "file" : "files");
	}
	else
	{
		fprintf(stderr, "Found %zu %s. Output piped to stdout.\n", foundFiles.size(), foundFiles.size() == 1 ? "file" : "files");
	}
}

void Sniffle::runGrep(const std::string& filePattern, const std::string& contentsPattern)
{
	// this can be done as a file find, plus additional contents search on the results.
	// TODO: contents search in multiple threads...

	std::vector<std::string> foundFiles;

	fprintf(stderr, "Searching for files...\n");

	if (!findFiles(filePattern, foundFiles, 0))
	{
		fprintf(stderr, "No files found matching file match criteria.\n");
		return;
	}

	fprintf(stderr, "Found %zu %s matching file match criteria.\n", foundFiles.size(), foundFiles.size() == 1 ? "file" : "files");

	bool printProgress = m_config.getPrintProgressWhenOutToStdOut() && !SystemHelpers::isStdOutATTY();

	FileGrepper grepper(m_config);

	bool foundPrevious = false;

	size_t totalFiles = foundFiles.size();
	size_t fileCount = 0;
	size_t lastPercentage = 101;
	size_t foundCount = 0;

	if (printProgress)
	{
		fprintf(stderr, "Grepping files for content...");
	}

	for (const std::string& fileItem : foundFiles)
	{
		if (printProgress)
		{
			fileCount++;
			size_t thisPercentage = (fileCount * 100) / totalFiles;
			if (thisPercentage != lastPercentage)
			{
				fprintf(stderr, "\rGrepping files for content - %zu%% complete...", thisPercentage);
				lastPercentage = thisPercentage;
			}
		}

		// the grepper itself does any printing...
		bool foundInFile = grepper.grepBasic(fileItem, contentsPattern, foundPrevious);

		if (foundInFile)
		{
			foundCount++;
		}

		// TODO: this bit could be kept within FileGrepper...
		foundPrevious |= foundInFile;
	}

	if (printProgress)
	{
		// annoyingly, we need to clear the remainder of previous progress line here, hence the space padding...
		fprintf(stderr, "\rFound content in %zu %s. Output piped to stdout.%-5s\n", foundCount, foundCount == 1 ? "file" : "files", " ");
	}
	else
	{
		if (SystemHelpers::isStdOutATTY())
		{
			fprintf(stderr, "Found content in %zu %s.\n", foundCount, foundCount == 1 ? "file" : "files");
		}
		else
		{
			fprintf(stderr, "Found content in %zu %s. Output piped to stdout.\n", foundCount, foundCount == 1 ? "file" : "files");
		}
	}
}

void Sniffle::runMatch(const std::string& filePattern, const std::string& contentsPattern)
{
	// this can be done as a file find, plus additional contents search on the results.
	// TODO: contents search in multiple threads...

	std::vector<std::string> foundFiles;

	fprintf(stderr, "Searching for files...\n");

	if (!findFiles(filePattern, foundFiles, 0))
	{
		fprintf(stderr, "No files found matching file match criteria.\n");
		return;
	}

	fprintf(stderr, "Found %zu %s matching file match criteria.\n", foundFiles.size(), foundFiles.size() == 1 ? "file" : "files");

	bool printProgress = m_config.getPrintProgressWhenOutToStdOut() && !SystemHelpers::isStdOutATTY();

	FileGrepper grepper(m_config);

	if (!grepper.initMatch(contentsPattern))
	{
		fprintf(stderr, "Invalid match pattern. Make sure you provide multiple items to search for, separated by the respective OR and AND character separators.\n");
		return;
	}

	bool foundPrevious = false;

	size_t totalFiles = foundFiles.size();
	size_t fileCount = 0;
	size_t lastPercentage = 101;
	size_t foundCount = 0;

	if (printProgress)
	{
		fprintf(stderr, "Searching files for match content...");
	}

	for (const std::string& fileItem : foundFiles)
	{
		if (printProgress)
		{
			fileCount++;
			size_t thisPercentage = (fileCount * 100) / totalFiles;
			if (thisPercentage != lastPercentage)
			{
				fprintf(stderr, "\rSearching files for match content - %zu%% complete...", thisPercentage);
				lastPercentage = thisPercentage;
			}
		}

		// the grepper itself does any printing...
		bool foundInFile = grepper.matchBasic(fileItem, foundPrevious);

		if (foundInFile)
		{
			foundCount++;
		}

		// TODO: this bit could be kept within FileGrepper...
		foundPrevious |= foundInFile;
	}

	if (printProgress)
	{
		// annoyingly, we need to clear the remainder of previous progress line here, hence the space padding...
		fprintf(stderr, "\rFound content in %zu %s. Output piped to stdout.%-5s\n", foundCount, foundCount == 1 ? "file" : "files", " ");
	}
	else
	{
		if (SystemHelpers::isStdOutATTY())
		{
			fprintf(stderr, "Found content in %zu %s.\n", foundCount, foundCount == 1 ? "file" : "files");
		}
		else
		{
			fprintf(stderr, "Found content in %zu %s. Output piped to stdout.\n", foundCount, foundCount == 1 ? "file" : "files");
		}
	}
}

//

Sniffle::PatternSearch Sniffle::classifyPattern(const std::string& pattern)
{
	Sniffle::PatternSearch result;

	std::vector<std::string> patternTokens;

	// first of all, work out if the path is an absolute full path, or a relative one.
	if (pattern[0] == '/')
	{
		// it was an absolute path, so we can just tokenise the path
		result.baseSearchPath = "/";

		StringHelpers::split(pattern, patternTokens, "/");
	}
	else
	{
		// it was a relative path, so attempt to work out what the full absolute path should be...

		// TODO: could try and use realpath() for this, but we'd still have to remove any wildcard items before
		//       we can use it, so for the moment, just do the obvious stuff ourselves...

		char szCurrDir[2048];
		if (getcwd(szCurrDir, 2048) == NULL)
		{
			// we couldn't get the current directory for some reason...
			fprintf(stderr, "Error: couldn't get current working directory.\n");
			result.type = ePatternError;
			return result;
		}

		std::string currentDir(szCurrDir);

		if (pattern[0] == '*' && pattern.find("/") == std::string::npos)
		{
			// we're likely just a file match pattern...
			result.baseSearchPath = currentDir;
			result.fileMatch = pattern;
			result.type = ePatternSimple;
			return result;
		}
		else if (pattern.find("/") != std::string::npos)
		{
			// for the moment, just slap the currentDir before what we have in the hope it will work...
			std::string expandedPath = FileHelpers::combinePaths(currentDir, pattern);

			result.baseSearchPath = "/";

			StringHelpers::split(expandedPath, patternTokens, "/");
		}
		else
		{
			fprintf(stderr, "Other relative path...\n");
			result.type = ePatternError;
			return result;
		}
	}

	// currently we only support one wildcard directory

	bool foundDirWildcard = false;
	bool collapseRemainderDirs = true; // collapse remainder directories into one item if possible
	std::string remainderDirsItem;

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
			// it should be the file filter

			result.fileMatch = token;

			if (collapseRemainderDirs && !remainderDirsItem.empty())
			{
				result.dirRemainders.push_back(remainderDirsItem);
			}
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
				if (collapseRemainderDirs)
				{
					remainderDirsItem = FileHelpers::combinePaths(remainderDirsItem, token);
				}
				else
				{
					// otherwise, add it to the remainders path to be looked for after the directory wildcard
					result.dirRemainders.push_back(token);
				}
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
	PatternSearch patternRes = classifyPattern(pattern);

	if (!configureFilenameMatcher(patternRes))
	{
		fprintf(stderr, "Couldn't understand search terms.\n");
		return false;
	}

	if (patternRes.type == ePatternSimple)
	{
		// just do a simple recursive search

		bool foundOK = getRelativeFilesInDirectoryRecursive(patternRes.baseSearchPath, "", 0, foundFiles);

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
		if (!FileHelpers::getDirectoriesInDirectory(patternRes.baseSearchPath, patternRes.dirWildcardMatch, m_config.getIgnoreHiddenDirectories(), wildCardDirs))
		{
			return false;
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

			// Note: these might be collapsed into a single item (done within classifyPattern() )

			for (const std::string& remainderDir : patternRes.dirRemainders)
			{
				testDir = FileHelpers::combinePaths(testDir, remainderDir);

				dir = opendir(testDir.c_str());

				if (!dir)
				{
					// it doesn't exist, so no point continuing with this one...
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
			//       spawing off threads to do further file globbing needs care... It *might* make some sense over NFS though,
			//       especially where symlinks are involved pointing to other file nodes...

			bool foundOK = getRelativeFilesInDirectoryRecursive(remainderFullDir, remainderFullDir, 0, foundFiles);
			foundSomeFiles |= foundOK;
		}

		return foundSomeFiles;
	}

	return false;
}

bool Sniffle::getRelativeFilesInDirectoryRecursive(const std::string& searchDirectoryPath, const std::string& relativeDirectoryPath,
													unsigned int currentDepth, std::vector<std::string>& files) const
{
	// Note: opendir() is used on purpose here, as scandir() and lsstat() don't reliably support S_ISLNK on symlinks over NFS,
	//       whereas opendir() allows this robustly. opendir() is also more efficient when operating on items one at a time...

	DIR* dir = opendir(searchDirectoryPath.c_str());
	if (!dir)
		return false;

	struct dirent* dirEnt = NULL;
	char tempBuffer[4096];

	while ((dirEnt = readdir(dir)) != NULL)
	{
		if (dirEnt->d_type == DT_DIR)
		{
			// if we're at the max depth already, don't continue...
			if (currentDepth >= m_config.getDirectoryRecursionDepth())
				continue;

			// if required, ignore hidden (starting with '.') directories
			if (m_config.getIgnoreHiddenDirectories() && strncmp(dirEnt->d_name, ".", 1) == 0)
				continue;

			// ignore built-in items
			if (strcmp(dirEnt->d_name, ".") == 0 || strcmp(dirEnt->d_name, "..") == 0)
				continue;

			// build up next directory level relative path
			std::string newFullDirPath = FileHelpers::combinePaths(searchDirectoryPath, dirEnt->d_name);
			std::string newRelativeDirPath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
			getRelativeFilesInDirectoryRecursive(newFullDirPath, newRelativeDirPath, currentDepth + 1, files);
		}
		else if (dirEnt->d_type == DT_LNK)
		{
			// if preemptive symlink skipping is enabled, see if we can skip the path without having to read the link
			// or do an expensive stat() call...
			if (m_config.getPreEmptiveSymlinkSkipping() && m_pFilenameMatcher->canSkipPotentialSymlinkFile(dirEnt->d_name))
			{
				// we can skip it
				continue;
			}

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
				// on the assumption that the target of the symlink is not another symlink (if so, this won't work reliably over NFS)
				// check what type it is
				struct stat statState;
				int ret = lstat(tempBuffer, &statState);

				// TODO: there's an assumption in the code in this block that the target of the link will
				//       have a similar filename (especially file extension) that the original symlink file does.
				//       If that's not the case, it will not do the correct thing.

				if (ret == -1)
				{
					// it's very likely a dead/broken/stale symlink pointing to a non-existent file...
					// ignore for the moment...
					// TODO: verbose log output option?
					continue;
				}
				else if (S_ISDIR(statState.st_mode))
				{
					// if we're at the max depth already, don't continue...
					if (currentDepth >= m_config.getDirectoryRecursionDepth())
						continue;

					// if required, ignore hidden (starting with '.') directories
					if (m_config.getIgnoreHiddenDirectories() && strncmp(tempBuffer, ".", 1) == 0)
						continue;

					// build up next directory level relative path
					std::string newFullDirPath = tempBuffer;
					std::string newRelativeDirPath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
					getRelativeFilesInDirectoryRecursive(newFullDirPath, newRelativeDirPath, currentDepth + 1, files);
				}
				else if (S_ISREG(statState.st_mode))
				{
					if (m_pFilenameMatcher->doesMatch(dirEnt->d_name))
					{
						std::string fullRelativePath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
						files.push_back(fullRelativePath);
					}
				}
				else
				{
					// it's some other type...
					continue;
				}
			}
		}
		else if (dirEnt->d_type == DT_REG)
		{
			// it's a file
			// see if it's what we want...

			// if required, ignore hidden (starting with '.') files
			if (m_config.getIgnoreHiddenFiles() && strncmp(dirEnt->d_name, ".", 1) == 0)
				continue;

			if (m_pFilenameMatcher->doesMatch(dirEnt->d_name))
			{
				std::string fullRelativePath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
				files.push_back(fullRelativePath);
			}
		}
		else
		{
			// we don't know what type it is, so
			// check stat() to see what it really is...
			// This generally seems to happen when the symlink points to a file on a different server...

			// if preemptive symlink skipping is enabled, see if we can skip the path without having to read the link
			// or do an expensive stat() call...
			if (m_config.getPreEmptiveSymlinkSkipping() && m_pFilenameMatcher->canSkipPotentialSymlinkFile(dirEnt->d_name))
			{
				// we can skip it
				continue;
			}

			struct stat statState;
			std::string newRelativePath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
			int ret = lstat(newRelativePath.c_str(), &statState);

			if (ret == -1)
			{
				// ignore for the moment...
				// it's very likely a dead/broken/stale symlink pointing to a non-existent file..
				// TODO: verbose log output option?
				continue;
			}
			else if (S_ISREG(statState.st_mode))
			{
				// it's a file
				// if required, ignore hidden (starting with '.') files
				if (m_config.getIgnoreHiddenFiles() && strncmp(dirEnt->d_name, ".", 1) == 0)
					continue;

				if (m_pFilenameMatcher->doesMatch(dirEnt->d_name))
				{
					std::string fullRelativePath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
					files.push_back(fullRelativePath);
				}
			}
			else if (S_ISDIR(statState.st_mode))
			{
				// if we're at the max depth already, don't continue...
				if (currentDepth >= m_config.getDirectoryRecursionDepth())
					continue;

				// if required, ignore hidden (starting with '.') directories
				if (m_config.getIgnoreHiddenDirectories() && strncmp(dirEnt->d_name, ".", 1) == 0)
					continue;

				// TODO: not sure this is right...
				getRelativeFilesInDirectoryRecursive(newRelativePath, newRelativePath, currentDepth + 1, files);
			}
			else
			{
				// not sure what's happened here...
				// another symlink?
				continue;
			}
		}
	}

	closedir(dir);

	return !files.empty();
}
