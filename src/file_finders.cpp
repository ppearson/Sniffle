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

#define EXTRA_DEBUG_OPENEDIRS 0

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

void FileFinder::setFilterParameters(const FilterParameters& filterParams)
{
	m_filter = filterParams;
}

bool FileFinder::getRelativeFilesInDirectoryRecursive(const std::string& searchDirectoryPath, const std::string& relativeDirectoryPath,
													unsigned int currentDepth, std::vector<std::string>& files) const
{
	// Note: opendir() is used on purpose here, as scandir() and lsstat() don't reliably support S_ISLNK on symlinks over NFS,
	//       whereas opendir() allows this robustly with d_type (in most cases). opendir() is also more efficient when operating on items one at a time...

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
			// if preemptive skipping is enabled, see if we can skip the path without having to read the link
			// or do an expensive stat() call...
			if (m_config.getPreEmptiveSkipping() && m_pFilenameMatcher->canSkipPotentialFile(dirEnt->d_name))
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
				// TODO: do we want lstat() here?
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
					// if required, ignore hidden (starting with '.') files
					if (m_config.getIgnoreHiddenFiles() && strncmp(dirEnt->d_name, ".", 1) == 0)
						continue;

					if (m_filter.getFilterTypeFlags() != 0)
					{
						if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILEMODIFIEDDATE_OLDER) &&
							 statState.st_mtime > m_filter.getModifiedTimestampThreshold())
						{
							continue;
						}
						else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILEMODIFIEDDATE_YOUNGER) &&
							 statState.st_mtime < m_filter.getModifiedTimestampThreshold())
						{
							continue;
						}
						else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILESIZE_LARGER) &&
							 statState.st_size < m_filter.getFileSizeThreshold())
						{
							continue;
						}
						else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILESIZE_SMALLER) &&
							 statState.st_size > m_filter.getFileSizeThreshold())
						{
							continue;
						}
					}

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

			// do the filename comparison first before any stat() for the filters, on the assumption it will be cheaper
			// and might allow us to skip the need to stat() files.
			if (!m_pFilenameMatcher->doesMatch(dirEnt->d_name))
				continue;

			if (m_filter.getFilterTypeFlags() != 0)
			{
				// if we need to do filtering, we need to stat the file to get the full details...

				std::string fullRelativePath = FileHelpers::combinePaths(searchDirectoryPath, dirEnt->d_name);

				struct stat statState;
				int ret = stat(fullRelativePath.c_str(), &statState);
				if (ret == -1)
				{
					// error.
					// TODO: something more appropriate?
					continue;
				}

				if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILEMODIFIEDDATE_OLDER) &&
					 statState.st_mtime > m_filter.getModifiedTimestampThreshold())
				{
					continue;
				}
				else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILEMODIFIEDDATE_YOUNGER) &&
					 statState.st_mtime < m_filter.getModifiedTimestampThreshold())
				{
					continue;
				}
				else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILESIZE_LARGER) &&
					 statState.st_size < m_filter.getFileSizeThreshold())
				{
					continue;
				}
				else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILESIZE_SMALLER) &&
					 statState.st_size > m_filter.getFileSizeThreshold())
				{
					continue;
				}
			}

			std::string fullRelativePath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
			files.push_back(fullRelativePath);
		}
		else if (dirEnt->d_type == DT_UNKNOWN)
		{
			// we don't know what type it is.
			// This situation can happen on older XFS filesystems and NFS mounts.
			// the struct dirent d_type entry can validly be DT_UNKNOWN, in which case we really need to
			// perform a stat() to work out what type it is (file, symlink, directory).
			// However, this is pretty expensive, so as an optional (but default) optimisation, we can attempt
			// to first see if we can skip the item completely, based off its name.

			// if preemptive skipping is enabled, see if we can skip the path without having to read the link
			// or do an expensive stat() call...
			if (m_config.getPreEmptiveSkipping() && m_pFilenameMatcher->canSkipPotentialFile(dirEnt->d_name))
			{
				// we can skip it
				continue;
			}

			struct stat statState;
			std::string newRelativePath = FileHelpers::combinePaths(relativeDirectoryPath, dirEnt->d_name);
			// Note: we explicitly call lstat() here, on the assumption it *might* be a symlink, in which
			//       case using lstat() saves us a readlink() in that case.
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

				if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILEMODIFIEDDATE_OLDER) &&
					 statState.st_mtime > m_filter.getModifiedTimestampThreshold())
				{
					continue;
				}
				else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILEMODIFIEDDATE_YOUNGER) &&
					 statState.st_mtime < m_filter.getModifiedTimestampThreshold())
				{
					continue;
				}
				else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILESIZE_LARGER) &&
					 statState.st_size < m_filter.getFileSizeThreshold())
				{
					continue;
				}
				else if ((m_filter.getFilterTypeFlags() & FilterParameters::FILTER_FILESIZE_SMALLER) &&
					 statState.st_size > m_filter.getFileSizeThreshold())
				{
					continue;
				}

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
				// TODO: handle this correctly, at least for symlinks, although need to work out what lstat()
				//       does in that case...
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

bool FileFinderBasicRecursive::findFiles(std::vector<std::string>& foundFiles)
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

bool FileFinderBasicRecursiveDirectoryWildcard::findFiles(std::vector<std::string>& foundFiles)
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

#if EXTRA_DEBUG_OPENEDIRS
		DIR* dir = opendir(testDir.c_str());
		if (!dir)
		{
			fprintf(stderr, "Error: 55\n");
			continue;
		}
		closedir(dir);
#else
		DIR* dir;
#endif

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

//

FileFinderBasicRecursiveDirectoryWildcardParallel::FileFinderBasicRecursiveDirectoryWildcardParallel(const Config& config,
								const FilenameMatcher* pFilenameMatcher,
								const PatternSearch& patternSearch) :
										FileFinder(config, pFilenameMatcher, patternSearch)
{
	
}

bool FileFinderBasicRecursiveDirectoryWildcardParallel::findFiles(std::vector<std::string>& foundFiles)
{
	// for this type of search, we look for directories matching the wildcard (currently just *) as a first step

	std::vector<std::string> wildCardDirs;
	if (!FileHelpers::getDirectoriesInDirectory(m_patternSearch.baseSearchPath, m_patternSearch.dirWildcardMatch, m_config.getIgnoreHiddenDirectories(), wildCardDirs))
	{
		return false;
	}
	
	unsigned int threads = std::min(m_config.getFindThreads(), (unsigned int)wildCardDirs.size());
	
	std::vector<std::vector<std::string> > aThreadFoundFiles;
	// one std::vector<std::string> per subdir/task
	// TODO: might be worth thinking about making these per-thread instead, but doing it this way will likely
	//       be useful in the future for custom file output to files named based on first-level subdirectories.
	aThreadFoundFiles.resize(wildCardDirs.size());

	// we have some first level directories where the wildcard is, so for each of those try and find remainder directories within each
	unsigned int count = 0;
	for (const std::string& wildcardDir : wildCardDirs)
	{
		WildcardDirTask* pNewTask = new WildcardDirTask(wildcardDir, aThreadFoundFiles[count++]);
		
		addTask(pNewTask);
	}
	
	start(threads);
		
	// now add
	for (const std::vector<std::string>& threadResults : aThreadFoundFiles)
	{
		foundFiles.insert(foundFiles.end(), threadResults.begin(), threadResults.end());
	}

	return !foundFiles.empty();
}

void FileFinderBasicRecursiveDirectoryWildcardParallel::processTask(Task* pTask)
{
	WildcardDirTask* pWildcardTask = static_cast<WildcardDirTask*>(pTask);
	
	std::string testDir = FileHelpers::combinePaths(m_patternSearch.baseSearchPath, pWildcardTask->m_dirItem);

#if EXTRA_DEBUG_OPENEDIRS
	DIR* dir = opendir(testDir.c_str());
	if (!dir)
	{
		fprintf(stderr, "Error: 55\n");
		continue;
	}
	closedir(dir);
#else
	DIR* dir;
#endif

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

	if (dirToCheck)
	{
		// otherwise, we should have a final directory matching the pattern, including the directory wildcard.
		// so now do a file search at that level
		
		std::vector<std::string>& foundFiles = pWildcardTask->m_foundFiles;
		getRelativeFilesInDirectoryRecursive(remainderFullDir, remainderFullDir, 0, foundFiles);
	}

	delete pWildcardTask;
}
