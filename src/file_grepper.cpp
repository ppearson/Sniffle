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

FileGrepper::FileGrepper()
{
	
}

bool FileGrepper::findBasic(const std::string& filename, const std::string& searchString, const OutputSettings& settings)
{
	// slow and basic search...
	
	std::ios::openmode mode = std::ios::in;

	std::fstream fileStream(filename.c_str(), mode);
	
	bool found = false;
	
	unsigned int printCount = 0;

	if (fileStream.fail())
	{
		return false;
	}
	else
	{
		std::string line;
		char buf[2048];

		while (fileStream.getline(buf, 2048))
		{
			line.assign(buf);
			
			if (line.find(searchString) != std::string::npos)
			{
				// we found the string

				if (settings.outputFilename)
				{
					if (settings.outputContentLines)
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
						}
					}					
				}
				
				found = true;

				if ((!settings.firstOnly && settings.additionalAfterLines == 0) || !settings.outputContentLines)
				{
					// we can break out.
					break;
				}
			}
			else if (found && settings.additionalAfterLines > 0)
			{
				// if we've found it before, we need to output additional lines...
			}
		}

	}
	fileStream.close();
	
	return found;
}
