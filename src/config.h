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

	enum ParseResult
	{
		eParseError,
		eParseNoneHandled,
		eParseHandledOK,
		eParseHelpWanted
	};

	void loadConfigFile();

	ParseResult parseArgs(int argc, char** argv, int startOptionArg, int& nextArgIndex);


	// getters

	unsigned int getGrepThreads() const
	{
		return m_grepThreads;
	}

	bool getPrintProgressWhenOutToStdOut() const
	{
		return m_printProgressWhenOutToStdOut;
	}

	unsigned int getDirectoryRecursionDepth() const
	{
		return m_directoryRecursionDepth;
	}

	bool getIgnoreHiddenFiles() const
	{
		return m_ignoreHiddenFiles;
	}

	bool getIgnoreHiddenDirectories() const
	{
		return m_ignoreHiddenDirectories;
	}

	bool getPreEmptiveSymlinkSkipping() const
	{
		return m_preEmptiveSymlinkSkipping;
	}

	int getMatchCount() const
	{
		return m_matchCount;
	}

	bool getFlushOutput() const
	{
		return m_flushOutput;
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

	char getMatchItemOrSeperatorChar() const
	{
		return m_matchItemOrSeperatorChar;
	}

	char getMatchItemAndSeperatorChar() const
	{
		return m_matchItemAndSeperatorChar;
	}

private:
	// for config file
	static bool getKeyValue(const std::string& configLine, std::string& key, std::string& value);

	// for command line args
	static bool getKeyValueFromArg(const std::string& argString, std::string& key, std::string& value);

	bool applyKeyValueSetting(const std::string& key, const std::string& value);

	static bool getBooleanValueFromString(const std::string& value);


private:

	// TODO: this isn't very scalable (but does it need to be?) - maybe key/value lookups instead?

	unsigned int	m_grepThreads;

	bool			m_printProgressWhenOutToStdOut; // if we think stdout is not a tty (so being piped), print progress via stderr

	int				m_directoryRecursionDepth;

	bool			m_ignoreHiddenFiles;
	bool			m_ignoreHiddenDirectories;

	bool			m_preEmptiveSymlinkSkipping; // whether to attempt to skip statting / following a symlink based on its likely filenames

	int				m_matchCount;

	bool			m_flushOutput; // flush output after each atomic print item (i.e. file)

	bool			m_outputFilename; // TODO: relative / absolute options as well?
	bool			m_outputContentLines; // whether to output the content where the match takes place

	bool			m_outputLineNumbers;
	bool			m_outputFileSize;

	// note: there's an implicit 'context' lines option here, which if set, will set both of the below
	unsigned int	m_beforeLines; // lines before
	unsigned int	m_afterLines; // lines after

	bool			m_blankLinesBetweenFiles; //

	char			m_matchItemOrSeperatorChar; //
	char			m_matchItemAndSeperatorChar; //

	std::string		m_outputToFile; // if non-empty, write output to this file
};

#endif // CONFIG_H
