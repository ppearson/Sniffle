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
#include "file_finders.h"

Sniffle::Sniffle() :
	m_pFilenameMatcher(nullptr),
	m_pFileFinder(nullptr)
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
	
	if (m_pFileFinder)
	{
		delete m_pFileFinder;
		m_pFileFinder = nullptr;
	}
}

bool Sniffle::configureGlobals()
{
	setlocale(LC_ALL, "C");
//	char const* previousLocale = setlocale(LC_ALL, "C");
//	fprintf(stderr, "Prev locale: %s\n", previousLocale);

	return true;
}

Config::ParseResult Sniffle::parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex)
{
	return m_config.parseArgs(argc, argv, startOptionArg, nextArgIndex);
}

bool Sniffle::parseFilter(int argc, char** argv, int startOptionArg, int& nextArgIndex)
{
	int lastProcessedArg = startOptionArg;

	bool error = false;

	for (int i = startOptionArg; i < argc; i++)
	{
		if (argv[i][0] != '-')
		{
			continue;
		}

		lastProcessedArg ++;

		// otherwise, we want to process it...
		std::string argString(argv[i]);

		if (argString == "-filefilter-moddate" ||
			argString == "-ff-m")
		{
			const char firstOp = argv[i + 1][0];

			// we can't easily use '<' and '>' chars here due to their piping semantics in the shell which is a bit annoying...
			if (firstOp != 'o' && firstOp != 'y')
			{
				error = true;

				lastProcessedArg ++;

				continue;
			}

			std::string nextArg(argv[i + 1]);

			bool younger = firstOp == 'y';

			// check that the next char is not null
			if (argv[i + 1][1] == 0)
			{
				// don't have enough items in the argument
				error = true;

				lastProcessedArg ++;

				continue;
			}

			std::string remainder = nextArg.substr(1);

			// currently we just support single char unit at the end of string, so...
			std::string amountStr = remainder.substr(0, remainder.size() - 1);
			unsigned int amountValueInHours = atoi(amountStr.c_str());
			std::string unit = remainder.substr(remainder.size() - 1, 1);

			// <4d
			// >12h
			if (unit == "d")
			{
				amountValueInHours *= 24;
			}

			m_filter.setFileModifiedDateFilter(younger, amountValueInHours);

			i++;

			lastProcessedArg ++;
		}
	}

	nextArgIndex = lastProcessedArg;

	return !error;
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
		fprintf(stderr, "\nFound %s %s.\n", StringHelpers::formatNumberThousandsSeparator(foundFiles.size()).c_str(),
				foundFiles.size() == 1 ? "file" : "files");
	}
	else
	{
		fprintf(stderr, "Found %s %s. Output piped to stdout.\n", StringHelpers::formatNumberThousandsSeparator(foundFiles.size()).c_str(),
				foundFiles.size() == 1 ? "file" : "files");
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

	fprintf(stderr, "Found %s %s matching file match criteria.\n", StringHelpers::formatNumberThousandsSeparator(foundFiles.size()).c_str(),
			foundFiles.size() == 1 ? "file" : "files");

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
			fprintf(stderr, "Found content in %s %s.\n", StringHelpers::formatNumberThousandsSeparator(foundCount).c_str(),
					foundCount == 1 ? "file" : "files");
		}
		else
		{
			fprintf(stderr, "Found content in %s %s. Output piped to stdout.\n", StringHelpers::formatNumberThousandsSeparator(foundCount).c_str(),
					foundCount == 1 ? "file" : "files");
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

	fprintf(stderr, "Found %s %s matching file match criteria.\n", StringHelpers::formatNumberThousandsSeparator(foundFiles.size()).c_str(),
			foundFiles.size() == 1 ? "file" : "files");

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
			fprintf(stderr, "Found content in %s %s.\n", StringHelpers::formatNumberThousandsSeparator(foundCount).c_str(),
					foundCount == 1 ? "file" : "files");
		}
		else
		{
			fprintf(stderr, "Found content in %s %s. Output piped to stdout.\n", StringHelpers::formatNumberThousandsSeparator(foundCount).c_str(),
					foundCount == 1 ? "file" : "files");
		}
	}
}

//

PatternSearch Sniffle::classifyPattern(const std::string& pattern)
{
	PatternSearch result;

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
			result.type = PatternSearch::ePatternError;
			return result;
		}

		std::string currentDir(szCurrDir);

		if (pattern[0] == '*' && pattern.find("/") == std::string::npos)
		{
			// we're likely just a file match pattern...
			result.baseSearchPath = currentDir;
			result.fileMatch = pattern;
			result.type = PatternSearch::ePatternSimple;
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
			result.type = PatternSearch::ePatternError;
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
			result.type = PatternSearch::ePatternWildcardDir;
		}
		else
		{
			result.type = PatternSearch::ePatternSimple;
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
		m_pFilenameMatcher = new FilenameMatcherExtension("*");
		return true;
	}

	size_t sepPos = pattern.fileMatch.find(".");
	if (sepPos == std::string::npos)
	{
		m_pFilenameMatcher = new FilenameMatcherExtension("*");
		return true;
	}

	std::string filenameCorePart = pattern.fileMatch.substr(0, sepPos);
	std::string extensionPart = pattern.fileMatch.substr(sepPos + 1);

	// simple extension only filter
	if (extensionPart.find("*", sepPos) == std::string::npos && filenameCorePart == "*")
	{
		// just a simple filename extension filter
		m_pFilenameMatcher = new FilenameMatcherExtension(extensionPart);
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

bool Sniffle::configureFileFinder(const PatternSearch& pattern)
{
	// this shouldn't be needed, as we shouldn't get to here if we don't have a valid pattern type, but...
	if (pattern.type == PatternSearch::ePatternError || pattern.type == PatternSearch::ePatternUnknown)
		return false;
	
	if (pattern.type == PatternSearch::ePatternSimple)
	{
		m_pFileFinder = new FileFinderBasicRecursive(m_config, m_pFilenameMatcher, pattern);
	}
	else if (pattern.type == PatternSearch::ePatternWildcardDir)
	{
		if (m_config.getFindThreads() == 1)
		{
			m_pFileFinder = new FileFinderBasicRecursiveDirectoryWildcard(m_config, m_pFilenameMatcher, pattern);
		}
		else
		{
			m_pFileFinder = new FileFinderBasicRecursiveDirectoryWildcardParallel(m_config, m_pFilenameMatcher, pattern);
		}
	}

	if (m_filter.getFilterType() != FileFinder::FilterParameters::eFilterNone)
	{
		m_pFileFinder->setFilterParameters(m_filter);
	}
	
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
	
	if (!configureFileFinder(patternRes))
	{
		// this shouldn't really trigger, as the above check should handle it currently, but...
		fprintf(stderr, "Couldn't understand search terms.\n");
		return false;
	}

	bool found = m_pFileFinder->findFiles(foundFiles);

	return found;
}
