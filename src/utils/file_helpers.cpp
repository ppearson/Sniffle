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

#include "file_helpers.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include <algorithm>
#include <set>

static const std::string kDirSep = "/";

FileHelpers::FileHelpers()
{

}

std::string FileHelpers::getFileExtension(const std::string& path)
{
	std::string extension;
	size_t dotPos = path.find_last_of('.');
	extension = path.substr(dotPos + 1);

	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

	return extension;
}

std::string FileHelpers::getFileDirectory(const std::string& path)
{
	std::string directory;
	size_t slashPos = path.find_last_of(kDirSep, path.length());
	if (slashPos != std::string::npos)
		directory = path.substr(0, slashPos + 1);

	return directory;
}

std::string FileHelpers::getFileName(const std::string& path)
{
	std::string fileName;
	size_t slashPos = path.find_last_of(kDirSep, path.length());
	if (slashPos != std::string::npos)
		fileName = path.substr(slashPos + 1);
	else
		return path;

	return fileName;
}

std::string FileHelpers::combinePaths(const std::string& path0, const std::string& path1)
{
	if (path0.empty())
		return path1;

	std::string final = path0;

	if (strcmp(final.substr(final.size() - 1, 1).c_str(), kDirSep.c_str()) != 0)
	{
		final += kDirSep;
	}

	final += path1;

	return final;
}

std::string FileHelpers::combinePaths(const std::vector<std::string>& pathItems)
{
	// current assumption is absolute path and that items don't have path separators themselves...
	std::string finalPath;

	for (const std::string& item : pathItems)
	{
		finalPath += kDirSep + item;
	}

	return finalPath;
}

bool FileHelpers::getDirectoriesInDirectory(const std::string& directoryPath, const std::string& dirMatch,
											bool ignoreHiddenDirs, std::vector<std::string>& directories)
{
	// Note: opendir() is used on purpose here, as scandir() and lsstat() don't reliably support S_ISLNK on symlinks over NFS,
	//       whereas opendir() allows this robustly with d_type (in most cases). opendir() is also more efficient when operating on items one at a time...
	DIR* dir = opendir(directoryPath.c_str());
	if (!dir)
		return false;

	struct dirent* dirEnt = NULL;
	char tempBuffer[4096];

	while ((dirEnt = readdir(dir)) != NULL)
	{
		// cope with symlinks by working out what they point at
		if (dirEnt->d_type == DT_LNK)
		{
			std::string fullAbsolutePath = combinePaths(directoryPath, dirEnt->d_name);
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
					// it's very likely a dead/broken/stale symlink pointing to a non-existent file...
					// ignore for the moment...
					continue;
				}
				else if (S_ISDIR(statState.st_mode))
				{
					// currently, we don't do any dirMatch checking, on the assumption it's only a full wildcard for the moment...
					directories.push_back(dirEnt->d_name);
				}
				else
				{
					// assume it's a file...
				}
			}
		}
		else if (dirEnt->d_type == DT_DIR)
		{
			// it's a directory

			// if required, ignore hidden (starting with '.') directories
			if (ignoreHiddenDirs && strncmp(dirEnt->d_name, ".", 1) == 0)
				continue;

			// ignore built-ins
			if (strcmp(dirEnt->d_name, ".") == 0 || strcmp(dirEnt->d_name, "..") == 0)
				continue;

			// currently, we don't do any dirMatch checking, on the assumption it's only a full wildcard for the moment...
			directories.push_back(dirEnt->d_name);
		}
		else if (dirEnt->d_type == DT_REG)
		{
			// a file, ignore it...
			continue;
		}
		else
		{
			// TODO: check stat to see what it really is...

			continue;
		}
	}

	closedir(dir);

	return !directories.empty();
}
