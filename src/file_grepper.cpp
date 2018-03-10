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

#include <fstream>

#include "config.h"

FileGrepper::FileGrepper(const Config& config) : m_config(config)
{
	
}

bool FileGrepper::findBasic(const std::string& filename, const std::string& searchString)
{
	// slow and basic search...
	
	std::ios::openmode mode = std::ios::in;
	
	// disable C++ sync with stdio (C) - this speeds up getline() quite a bit...
	// we can do this as we only use C++ stuff for file reading (currently),
	// and C stuff for output, so we don't care about synchronisation between the two
	std::ios::sync_with_stdio(false); 

	std::fstream fileStream(filename.c_str(), mode);
	
	bool found = false;
	
	unsigned int printCount = 0;

	if (fileStream.fail())
	{
		// for the moment, we don't want to report any errors for files we can't access (invalid permissions, etc)
		return false;
	}
	
	std::string line;
	char buf[2048];
	
	unsigned int lineCount = 0;

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
						fprintf(stderr, "%s:\n", filename.c_str());
					}
					
					fprintf(stderr, "%s\n", line.c_str());
				}
				else
				{
					if (!found)
					{
						// just the filename
						fprintf(stderr, "%s\n", filename.c_str());
						
						// we don't want the content lines, so we can just break out
						break;
					}
				}
			}
			
			found = true;

			if ((!m_config.getFirstResultOnly() && m_config.getAfterLines() == 0) || !m_config.getOutputContentLines())
			{
				// we can break out.
				break;
			}
		}
		else if (found && (printCount < m_config.getAfterLines()))
		{
			// if we've found it before, we need to output additional lines...
			fprintf(stderr, "%s\n", line.c_str());
			
			printCount++;
		}
		
		lineCount ++;
	}

	fileStream.close();
	
	return found;
}
