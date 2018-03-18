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

#include "file_grepper.h"

#include <string.h>

#include "string_helpers.h"

#include "config.h"

static const unsigned int kStringLength = 2048;

FileGrepper::FileGrepper(const Config& config) : m_config(config)
{
	m_readBufferSize = 4096;

	// disable C++ sync with stdio (C) - this can speed up getline() quite a bit...
	// we can do this as we only use C++ stuff for file reading (currently),
	// and C stuff for output, so we don't care about synchronisation between the two
	std::ios::sync_with_stdio(false);
	
	if (m_config.getBeforeLines() > 0)
	{
		m_stringBuffer.init(m_config.getBeforeLines(), kStringLength);
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

	unsigned int lineIndex = 0;

	while (fileStream.getline(buf, kStringLength))
	{
		const char* findI = strstr(buf, searchString.c_str());
		
		if (findI != NULL)
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

			if (m_config.getOutputContentLines())
			{
				fprintf(stdout, "%s\n", buf);
			}
			
			bool haveFoundEnoughItems = m_config.getMatchCount() != -1 && foundCount >= m_config.getMatchCount();

			if ((haveFoundEnoughItems && afterLinesToPrint == 0) || !m_config.getOutputContentLines())
			{
				// we can break out.
				break;
			}
		}
		else if (afterLinesToPrint > 0)
		{
			// if we've found it before, we need to output additional lines...
			fprintf(stdout, "%s\n", buf);

			afterLinesToPrint--;
		}

		lineIndex ++;
	}

	fileStream.close();

	return foundCount > 0;
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

	unsigned int lineIndex = 0;

	while (fileStream.getline(buf, kStringLength))
	{	
		bool foundAString = false;
		
		for (const std::string& matchString : m_aMatchItems)
		{
			if (strstr(buf, matchString.c_str()) != NULL)
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
	
					foundSomething = true;
					
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
					else
					{
						foundSomething = true;
					}
				}
			}

			if (m_config.getOutputContentLines())
			{
				fprintf(stdout, "%s\n", buf);
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
			fprintf(stdout, "%s\n", buf);

			afterLinesToPrint--;
		}

		lineIndex ++;
	}

	fileStream.close();

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
	
	unsigned int lineIndex = 0;
	
	// we start off looking for the first item...
	unsigned int lastItemToMatchIndex = m_aMatchItems.size() - 1;
	unsigned int itemToMatchIndex = 0;
	std::string itemToMatch = m_aMatchItems[0];

	while (fileStream.getline(buf, kStringLength))
	{
		if (strstr(buf, itemToMatch.c_str()) == NULL)
		{
			if (!foundAll)
			{
				// didn't find it, and haven't found the last item yet, so continue on to the next line
				lineIndex ++;
				continue;
			}
			else if (foundAll && afterLinesToPrint > 0)
			{
				sprintf(szTemp, "%s\n", buf);
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
			sprintf(szTemp, "%s\n", buf);
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

