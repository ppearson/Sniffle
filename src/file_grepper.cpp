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



#include "config.h"

FileGrepper::FileGrepper(const Config& config) : m_config(config)
{
	m_readBufferSize = 4096;

	// disable C++ sync with stdio (C) - this speeds up getline() quite a bit...
	// we can do this as we only use C++ stuff for file reading (currently),
	// and C stuff for output, so we don't care about synchronisation between the two
	std::ios::sync_with_stdio(false);
}

bool FileGrepper::initMatch(const std::string& matchString)
{
	return true;
}

bool FileGrepper::findBasic(const std::string& filename, const std::string& searchString, bool foundPreviousFile)
{
	// slow and basic search...
	std::fstream fileStream;
	if (!openFilestream(filename, fileStream))
		return false;

	bool found = false;

	unsigned int printCount = 0;

	std::string line;
	char buf[2048];

	unsigned int lineIndex = 0;

	while (fileStream.getline(buf, 2048))
	{
		line.assign(buf);

		if (line.find(searchString) != std::string::npos)
		{
			// we found the string

			if (m_config.getOutputFilename())
			{
				if (m_config.getOutputContentLines())
				{
					// the filename if it's the first time for this file
					if (!found)
					{
						if (foundPreviousFile && m_config.getBlankLinesBetweenFiles())
						{
							fprintf(stdout, "\n");
						}
						fprintf(stdout, "%s :\n", filename.c_str());

						found = true;
					}
				}
				else
				{
					if (!found)
					{
						// technically, we should do a new line if asked, but doesn't seem worth it if we're not outputting
						// the contents...

						// just the filename
						fprintf(stdout, "%s\n", filename.c_str());

						found = true;

						// we don't want the content lines, so we can just break out
						break;
					}
				}
			}

			if (m_config.getOutputContentLines())
			{
				fprintf(stdout, "%s\n", line.c_str());
			}

			if ((m_config.getFirstResultOnly() && m_config.getAfterLines() == 0) || !m_config.getOutputContentLines())
			{
				// we can break out.
				break;
			}
		}
		else if (found && (printCount < m_config.getAfterLines()))
		{
			// if we've found it before, we need to output additional lines...
			fprintf(stdout, "%s\n", line.c_str());

			printCount++;
		}

		lineIndex ++;
	}

	fileStream.close();

	return found;
}

bool FileGrepper::matchBasic(const std::string& filename, bool foundPreviousFile)
{
	return false;
}

bool FileGrepper::openFilestream(const std::string& filename, std::fstream& fileStream)
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
