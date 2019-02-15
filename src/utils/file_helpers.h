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
 ---------
*/

#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include <string>
#include <vector>

class FileHelpers
{
public:
	FileHelpers();

	static std::string getFileExtension(const std::string& path);
	static std::string getFileDirectory(const std::string& path);
	static std::string getFileName(const std::string& path);

	// for local filenames only (no paths)
	static std::string stripExtensionFromFilename(const std::string& filename);

	static std::string combinePaths(const std::string& path0, const std::string& path1);
	static std::string combinePaths(const std::vector<std::string>& pathItems);

	// this returns relative directory names to the first level
	// currently dirMatch has to be '*'...
	static bool getDirectoriesInDirectory(const std::string& directoryPath, const std::string& dirMatch,
										  bool ignoreHiddenDirs, std::vector<std::string>& directories);
};

#endif // FILE_HELPERS_H
