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

#include "file_finders.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h> // for readlink()
#include <string.h>

#include "config.h"
#include "filename_matchers.h"
#include "pattern.h"

#include "utils/file_helpers.h"

FileFinder::FileFinder(const Config& config,
                       const FilenameMatcher* pFilenameMatcher,
			           const PatternSearch& patternSearch) :
			m_config(config),
            m_pFilenameMatcher(pFilenameMatcher),
            m_patternSearch(patternSearch)
{
	
}

FileFinder::~FileFinder()
{
	
}

bool FileFinder::getRelativeFilesInDirectoryRecursive(const std::string& searchDirectoryPath, const std::string& relativeDirectoryPath,
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
		else if (dirEnt->d_type == DT_LNK && m_config.getFollowSymlinks())
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

//

FileFinderBasicRecursive::FileFinderBasicRecursive(const Config& config,
						 const FilenameMatcher* pFilenameMatcher,
						 const PatternSearch& patternSearch) :
				FileFinder(config, pFilenameMatcher, patternSearch)
{
	
}

bool FileFinderBasicRecursive::findFiles(std::vector<std::string>& foundFiles) const
{
	bool foundOK = getRelativeFilesInDirectoryRecursive(m_patternSearch.baseSearchPath, "", 0, foundFiles);

	if (foundOK)
	{
		// the results of the above are relative, so add the base search path

		for (std::string& item : foundFiles)
		{
			item = FileHelpers::combinePaths(m_patternSearch.baseSearchPath, item);
		}
	}

	return foundOK;
}

//

FileFinderBasicRecursiveDirectoryWildcard::FileFinderBasicRecursiveDirectoryWildcard(const Config& config,
						 const FilenameMatcher* pFilenameMatcher,
						 const PatternSearch& patternSearch) :
								FileFinder(config, pFilenameMatcher, patternSearch)
{
	
}

bool FileFinderBasicRecursiveDirectoryWildcard::findFiles(std::vector<std::string>& foundFiles) const
{
	// for this type of search, we look for directories matching the wildcard (currently just *) as a first step

	std::vector<std::string> wildCardDirs;
	if (!FileHelpers::getDirectoriesInDirectory(m_patternSearch.baseSearchPath, m_patternSearch.dirWildcardMatch, m_config.getIgnoreHiddenDirectories(), wildCardDirs))
	{
		return false;
	}

	bool foundSomeFiles = false;

	// we have some first level directories where the wildcard is, so for each of those try and find remainder directories within each
	for (const std::string& wildcardDir : wildCardDirs)
	{
		std::string testDir = FileHelpers::combinePaths(m_patternSearch.baseSearchPath, wildcardDir);

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

		for (const std::string& remainderDir : m_patternSearch.dirRemainders)
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
