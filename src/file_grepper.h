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

#ifndef FILE_GREPPER_H
#define FILE_GREPPER_H

#include <string>

class Config;

class FileGrepper
{
public:
	FileGrepper(const Config& config);
	
	
	
	// slow, basic
	bool findBasic(const std::string& filename, const std::string& searchString, bool foundPreviousFile);

private:
	const Config&	m_config;
};

#endif // FILE_GREPPER_H