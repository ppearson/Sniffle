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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config
{
public:
	Config();
	
	void loadConfigFile();
	
	bool parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex);
	
	
	// getters
	
	unsigned int getGrepThreads() const
	{
		return m_grepThreads;
	}
	
	bool getFirstResultOnly() const
	{
		return m_firstResultOnly;
	}
	
	bool getOutputFilename() const
	{
		return m_outputFilename;
	}
	
	bool getOutputContentLines() const
	{
		return m_outputContentLines;
	}
	
	bool getOutputLineNumbers() const
	{
		return m_outputLineNumbers;
	}
	
	unsigned int getBeforeLines() const
	{
		return m_beforeLines;
	}
	
	unsigned int getAfterLines() const
	{
		return m_afterLines;
	}
	
	
	bool getBlankLinesBetweenFiles() const
	{
		return m_blankLinesBetweenFiles;
	}
	
private:
	static bool getKeyValue(const std::string& configLine, std::string& key, std::string& value);
	
	
	
private:
	
	// TODO: this isn't very scalable (but does it need to be?) - maybe key/value lookups instead?
	
	unsigned int	m_grepThreads;
	
	bool			m_firstResultOnly;
	
	bool			m_outputFilename; // TODO: relative / absolute options as well?
	bool			m_outputContentLines; // whether to output the content where the match takes place
	
	bool			m_outputLineNumbers;
	
	// note: there's an implicit 'context' lines option here, which if set, will set both of the below
	unsigned int	m_beforeLines; // lines before
	unsigned int	m_afterLines; // lines after
	
	bool			m_blankLinesBetweenFiles; //
	
	std::string		m_searchItemSeperatorChar; //
	
	std::string		m_outputToFile; // if non-empty, write output to this file
};

#endif // CONFIG_H
