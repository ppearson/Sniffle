/*
 Sniffle
 Copyright 2018-2019 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------utils/string_helpers.cpp
*/

#include "file_grepper.h"

#include <string.h>

#include "utils/string_helpers.h"

#include "config.h"

// somewhat arbitrary, and obviously not perfect, but good enough for now...
static const unsigned int kStringLength = 2048;

FileGrepper::FileGrepper(const Config& config) : m_config(config),
	m_cacheBeforeLines(false),
	m_shortCircuit(false)
{
	m_readBufferSize = 4096;

	// disable C++ sync with stdio (C) - this can speed up getline() quite a bit...
	// we can do this as we only use C++ stuff for file reading (currently),
	// and C stuff for output, so we don't care about synchronisation between the two
	std::ios::sync_with_stdio(false);
	
	if (m_config.getBeforeLines() > 0)
	{
		m_cacheBeforeLines = true;
		// allocate one extra, as otherwise we can overwrite a previous line we need as the current line
		// we're processing currently occupies one buffer slot
		m_stringBuffer.init(m_config.getBeforeLines() + 1, kStringLength);
	}
	
	if (!m_config.getShortCircuitString().empty())
	{
		m_shortCircuit = true;
		m_shortCircuitString = m_config.getShortCircuitString();
	}
}

bool FileGrepper::initMatch(const std::string& matchString)
{
	// currently the assumption is all operators are the same, but in the future this could be extended
	// to support different ones and parentheses.
	
	if (matchString.find(m_config.getMatchItemOrSeperatorChar()) != std::string::npos)
	{
		m_matchType = eMatchTypeOr;
		std::string matchChar = "|";
		matchChar[0] = m_config.getMatchItemOrSeperatorChar();
		StringHelpers::split(matchString, m_aMatchItems, matchChar);
	}
	else if (matchString.find(m_config.getMatchItemAndSeperatorChar() != std::string::npos))
	{
		m_matchType = eMatchTypeAnd;
		std::string matchChar = "&";
		matchChar[0] = m_config.getMatchItemAndSeperatorChar();
		StringHelpers::split(matchString, m_aMatchItems, matchChar);
	}
	else
	{
		return false;
	}
	
	if (m_aMatchItems.empty())
		return false;
		
	return true;
}

bool FileGrepper::grepBasic(const std::string& filename, const std::string& searchString, bool foundPreviousFile)
{
	// slow and basic search...
	std::fstream fileStream;
	if (!openFileStream(filename, fileStream))
		return false;

	int foundCount = 0;

	unsigned int afterLinesToPrint = 0;

	char buf[kStringLength];

	unsigned int lineIndex = 1; // start at one as this value is only used for printing line number purposes

	// make a note of the list line we printed output lines for so we can prevent outputting lines multiple times
	// when context (before and after) content line output is enabled.
	unsigned int lastOutputContentLine = 0;

	while (fileStream.getline(buf, kStringLength))
	{
		if (m_cacheBeforeLines)
		{
			char* nextBeforeBuffer = m_stringBuffer.getNextBuffer();
			// TODO: we could get rid of this strcpy() if we did the fileStream.getline() directly into the nextBeforeBuffer itself...
			strcpy(nextBeforeBuffer, buf);
		}

		const char* findI = strstr(buf, searchString.c_str());
		
		if (findI != nullptr)
		{
			// we found the string
			
			if (m_config.getOutputFilename())
			{
				if (m_config.getOutputContentLines())
				{
					// the filename if it's the first time for this file
					if (foundCount == 0)
					{
						if (foundPreviousFile && m_config.getBlankLinesBetweenFiles())
						{
							fprintf(stdout, "\n");
						}
						fprintf(stdout, "%s :\n", filename.c_str());

						foundCount = 1;
					}
					else
					{
						foundCount ++;
					}
					
					// as we found the item, reset the after line content count item
					afterLinesToPrint = m_config.getAfterLines();
				}
				else
				{
					if (foundCount == 0)
					{
						// technically, we should do a new line if asked, but doesn't seem worth it if we're not outputting
						// the contents...

						// just the filename
						fprintf(stdout, "%s\n", filename.c_str());

						foundCount = 1;

						// we don't want the content lines, so we can just break out as we don't need to do anything else...
						break;
					}
					else
					{
						foundCount ++;
					}
				}
			}
			else
			{
				foundCount ++;
			}

			if (m_config.getOutputContentLines())
			{
				// see if we need to output before lines first
				if (m_cacheBeforeLines)
				{
					unsigned int minLine = lineIndex - 1; // lineIndex starts at 1, so can't be 0
					// check lastOutputContentLine to make sure we don't output lines multiple times
					unsigned int lastOutputLineDiff = minLine - lastOutputContentLine;
					minLine = std::min(minLine, lastOutputLineDiff);
					unsigned int beforeLinesToPrint = std::min(m_config.getBeforeLines(), minLine);

					for (unsigned int i = beforeLinesToPrint; i > 0; i--)
					{
						const char* prevBuffer = m_stringBuffer.getPreviousBuffer(i);

						if (m_config.getOutputLineNumbers())
						{
							fprintf(stdout, "%u: %s\n", lineIndex - i, prevBuffer);
						}
						else
						{
							fprintf(stdout, "%s\n", prevBuffer);
						}
					}
				}

				if (m_config.getOutputLineNumbers())
				{
					fprintf(stdout, "%u: %s\n", lineIndex, buf);
				}
				else
				{
					fprintf(stdout, "%s\n", buf);
				}

				lastOutputContentLine = lineIndex;
			}
			
			bool haveFoundEnoughItems = m_config.getMatchCount() != -1 && foundCount >= m_config.getMatchCount();

			if ((haveFoundEnoughItems && afterLinesToPrint == 0) || !m_config.getOutputContentLines())
			{
				// we can break out.
				break;
			}
		}
		else if (m_config.getOutputContentLines() && afterLinesToPrint > 0)
		{
			// if we've found it before, we need to output additional lines...
			if (m_config.getOutputLineNumbers())
			{
				fprintf(stdout, "%u: %s\n", lineIndex, buf);
			}
			else
			{
				fprintf(stdout, "%s\n", buf);
			}

			lastOutputContentLine = lineIndex;

			afterLinesToPrint--;
		}
		else if (m_shortCircuit)
		{
			const char* findI = strstr(buf, m_shortCircuitString.c_str());
			if (findI != nullptr)
			{
				break;
			}
		}

		lineIndex ++;
	}

	fileStream.close();

	if (foundCount > 0)
	{
		if (m_config.getFlushOutput())
		{
			fflush(stdout);
		}

		return true;
	}

	return false;
}

bool FileGrepper::countBasic(const std::string& filename, const std::string& searchString)
{
	// slow and basic search...
	std::fstream fileStream;
	if (!openFileStream(filename, fileStream))
		return false;
	
	unsigned int foundCount = 0;
	
	char buf[kStringLength];

	while (fileStream.getline(buf, kStringLength))
	{
		const char* findI = strstr(buf, searchString.c_str());
		
		if (findI == nullptr)
		{
			if (m_shortCircuit)
			{
				findI = strstr(buf, m_shortCircuitString.c_str());
				if (findI != nullptr)
				{
					break;
				}
			}
			continue;
		}
		
		// otherwise, we found the string
		foundCount ++;
	}

	fileStream.close();

	if (foundCount > 0)
	{
		// can't really think of a useful use-case where you wouldn't want the filename, but...
		if (m_config.getOutputFilename())
		{
			fprintf(stdout, "%u: %s\n", foundCount, filename.c_str());
		}
		else
		{
			fprintf(stdout, "%u\n", foundCount);
		}
		
		if (m_config.getFlushOutput())
		{
			fflush(stdout);
		}

		return true;
	}
	
	return false;
}

bool FileGrepper::matchBasic(const std::string& filename, bool foundPreviousFile)
{
	if (m_matchType == eMatchTypeOr)
	{
		return matchBasicOr(filename, foundPreviousFile);
	}
	else
	{
		return matchBasicAnd(filename, foundPreviousFile);
	}
}

bool FileGrepper::matchBasicOr(const std::string& filename, bool foundPreviousFile)
{
	std::fstream fileStream;
	if (!openFileStream(filename, fileStream))
		return false;
	
	// this "or" version basically just acts as a normal find which can look for multiple items (on different lines)
	// in any order.
	// Output is also a bit weird, as we output all lines for all items found.

	bool foundSomething = false;

	unsigned int afterLinesToPrint = 0;

	char buf[kStringLength];

	unsigned int lineIndex = 1; // start at one as this value is only used for printing line number purposes

	while (fileStream.getline(buf, kStringLength))
	{	
		bool foundAString = false;
		
		for (const std::string& matchString : m_aMatchItems)
		{
			if (strstr(buf, matchString.c_str()) != nullptr)
			{
				foundAString = true;
				break;
			}
		}
		
		if (foundAString)
		{
			// we found a string
			
			if (m_config.getOutputFilename())
			{
				if (m_config.getOutputContentLines())
				{
					// the filename if it's the first time for this file
					if (!foundSomething)
					{
						if (foundPreviousFile && m_config.getBlankLinesBetweenFiles())
						{
							fprintf(stdout, "\n");
						}
						fprintf(stdout, "%s :\n", filename.c_str());
					}
					
					// as we found the item, reset the after line content count item
					afterLinesToPrint = m_config.getAfterLines();
				}
				else
				{
					if (!foundSomething)
					{
						// technically, we should do a new line if asked, but doesn't seem worth it if we're not outputting
						// the contents...

						// just the filename
						fprintf(stdout, "%s\n", filename.c_str());

						foundSomething = true;

						// we don't want the content lines, so we can just break out as we don't need to do anything else...
						break;
					}
				}
			}

			foundSomething = true;

			if (m_config.getOutputContentLines())
			{
				if (m_config.getOutputLineNumbers())
				{
					fprintf(stdout, "%u: %s\n", lineIndex, buf);
				}
				else
				{
					fprintf(stdout, "%s\n", buf);
				}
			}
			
			// TODO: work out if we want to do anything about match counts? Not really sure how it would work... Match all equal number
			//       of times?
			
			if (!m_config.getOutputContentLines())
			{
				// we can break out.
				break;
			}
		}
		else if (afterLinesToPrint > 0)
		{
			// if we've found it before, we need to output additional lines...
			if (m_config.getOutputLineNumbers())
			{
				fprintf(stdout, "%u: %s\n", lineIndex, buf);
			}
			else
			{
				fprintf(stdout, "%s\n", buf);
			}

			afterLinesToPrint--;
		}
		else if (m_shortCircuit)
		{
			const char* findI = strstr(buf, m_shortCircuitString.c_str());
			if (findI != nullptr)
			{
				break;
			}
		}

		lineIndex ++;
	}

	fileStream.close();

	if (foundSomething)
	{
		if (m_config.getFlushOutput())
		{
			fflush(stdout);
		}
	}

	return foundSomething;
}

bool FileGrepper::matchBasicAnd(const std::string& filename, bool foundPreviousFile)
{
	std::fstream fileStream;
	if (!openFileStream(filename, fileStream))
		return false;
	
	// in contrast, this "and" version will only match files (and output their content) if
	// *all* string items are found - on separate lines - in order. Context/After/Before lines are only
	// printed for the final item.
	// Because of this behaviour, any output has to be deferred until a full match of all items is
	// found.
	
	// The assumption here is that there will be more than one item to match - if only one is given,
	// the code will do the wrong thing (grep type should be used instead).
	
	std::string finalOutput;
	
	char buf[kStringLength];
	
	bool foundAll = false;
	
	unsigned int afterLinesToPrint = 0;
	
	// temp buffer for formatting output
	char szTemp[kStringLength];
	
	unsigned int lineIndex = 1; // start at one as this value is only used for printing line number purposes
	
	// we start off looking for the first item...
	unsigned int lastItemToMatchIndex = m_aMatchItems.size() - 1;
	unsigned int itemToMatchIndex = 0;
	std::string itemToMatch = m_aMatchItems[0];

	while (fileStream.getline(buf, kStringLength))
	{
		if (strstr(buf, itemToMatch.c_str()) == nullptr)
		{
			if (!foundAll)
			{
				if (m_shortCircuit)
				{
					const char* findI = strstr(buf, m_shortCircuitString.c_str());
					if (findI != nullptr)
					{
						break;
					}
				}
				
				// didn't find it, and haven't found the last item yet, so continue on to the next line
				lineIndex ++;
				continue;
			}
			else if (foundAll && afterLinesToPrint > 0)
			{
				if (m_config.getOutputLineNumbers())
				{
					sprintf(szTemp, "%u: %s\n", lineIndex, buf);
				}
				else
				{
					sprintf(szTemp, "%s\n", buf);
				}
				finalOutput.append(szTemp);
				
				afterLinesToPrint --;
				lineIndex ++;
				continue;
			}
			else
			{
				break;
			}
		}
		
		// we did find it...
		
		// if we've found the first item, "print" the filename if required
		if (itemToMatchIndex == 0 && m_config.getOutputFilename())
		{
			if (foundPreviousFile && m_config.getBlankLinesBetweenFiles())
			{
				// start with a new line if it's the next file
				finalOutput = "\n";
			}
			if (m_config.getOutputContentLines())
			{
				// the filename if it's the first time for this file
				sprintf(szTemp, "%s :\n", filename.c_str());
			}
			else
			{
				// just the filename
				// technically, we should do a new line if asked, but doesn't seem worth it if we're not outputting
				// the contents...
				sprintf(szTemp, "%s\n", filename.c_str());
			}
			finalOutput.append(szTemp);
		}
		
		// now output the item itself if required. We output the line of all items matched.
		if (m_config.getOutputContentLines())
		{
			if (m_config.getOutputLineNumbers())
			{
				sprintf(szTemp, "%u: %s\n", lineIndex, buf);
			}
			else
			{
				sprintf(szTemp, "%s\n", buf);
			}
			finalOutput.append(szTemp);
		}
		
		if (itemToMatchIndex == lastItemToMatchIndex)
		{
			// this is the last one to look for, so we were successful in finding all items in order
			foundAll = true;
			
			// if we're the last one, we can break out if we don't need any after lines...
			if (m_config.getAfterLines() == 0)
			{
				break;
			}
			
			// otherwise, mark how many after lines we want to do
			afterLinesToPrint = m_config.getAfterLines();
		}
		else
		{
			// otherwise, move on to the next item to look for
			itemToMatchIndex ++;
			itemToMatch = m_aMatchItems[itemToMatchIndex];
		}
		
		lineIndex ++;
	}
	
	fileStream.close();
	
	if (foundAll)
	{
		fprintf(stdout, "%s", finalOutput.c_str());
		if (m_config.getFlushOutput())
		{
			fflush(stdout);
		}
	}
	
	return foundAll;
}

bool FileGrepper::openFileStream(const std::string& filename, std::fstream& fileStream)
{
	std::ios::openmode mode = std::ios::in;

//	char buffer[8*1024];
//	fileStream.rdbuf()->pubsetbuf(buffer, sizeof(buffer));

	fileStream.open(filename.c_str(), mode);

	if (!fileStream.is_open())
	{
		// for the moment, don't report any errors...
		return false;
	}

	if (fileStream.fail())
	{
		// for the moment, we don't want to report any errors for files we can't access (invalid permissions, etc)
		return false;
	}

	return true;
}

