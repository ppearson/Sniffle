/*
 Sniffle
 Copyright 2019 Peter Pearson.

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

#ifndef TESTS_H
#define TESTS_H

#include <stdio.h>

#include "utils/string_helpers.h"
#include "filename_matchers.h"

// disgusting putting this in header instead of doing it properly, but don't want to play
// around with build stuff yet...

class SniffleTests
{
public:
	bool testUtils()
	{
		return true;
	}
	
	
	
	bool testFilenameMatchers()
	{
		// test FilenameMatcherExtension
		
		FilenameMatcherExtension fm1("log");
		
		if (!CHECK_RETURN_TRUE("test wrong extension", fm1.canSkipPotentialFile("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm1.canSkipPotentialFile("mytest")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test correct extension", fm1.canSkipPotentialFile("mytest.log")))
			return false;
		
		//
		
		if (!CHECK_RETURN_FALSE("test wrong extension", fm1.doesMatch("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm1.doesMatch("mytest")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test correct extension", fm1.doesMatch("mytest.log")))
			return false;
		
		////////
		
		// test FilenameMatcherNameWildcard - match left
		FilenameMatcherNameWildcard fm2_1("mytes*", "log");
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_1.canSkipPotentialFile("mytest")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension", fm2_1.canSkipPotentialFile("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension and core filename", fm2_1.canSkipPotentialFile("test.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong core filename", fm2_1.canSkipPotentialFile("test.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong long core filename", fm2_1.canSkipPotentialFile("test_this_is_a_test_again.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong short core filename", fm2_1.canSkipPotentialFile("t.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and right core filename", fm2_1.canSkipPotentialFile("mytest.log")))
			return false;
		
		//
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_1.doesMatch("mytest")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension", fm2_1.doesMatch("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension and core filename", fm2_1.doesMatch("test.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong core filename", fm2_1.doesMatch("test.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong long core filename", fm2_1.doesMatch("test_this_is_a_test_again.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong short core filename", fm2_1.doesMatch("t.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and right core filename", fm2_1.doesMatch("mytest.log")))
			return false;
		
		//////////
		
		// test FilenameMatcherNameWildcard - match inner
		FilenameMatcherNameWildcard fm2_2("*es*", "log");
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_2.canSkipPotentialFile("mytest")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension", fm2_2.canSkipPotentialFile("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension and core filename", fm2_2.canSkipPotentialFile("tegst.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong core filename", fm2_2.canSkipPotentialFile("tegst.log")))
			return false;

		if (!CHECK_RETURN_TRUE("test right extension and wrong long core filename", fm2_2.canSkipPotentialFile("tegst_this_is_a_tegst_again.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong short core filename", fm2_2.canSkipPotentialFile("t.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and right core filename", fm2_2.canSkipPotentialFile("mytest.log")))
			return false;
		
		//
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_2.doesMatch("mytest")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension", fm2_2.doesMatch("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension and core filename", fm2_2.doesMatch("test.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong core filename", fm2_2.doesMatch("tegst.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong long core filename", fm2_2.doesMatch("tegst_this_is_a_tegst_again.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong short core filename", fm2_2.doesMatch("t.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and right core filename", fm2_2.doesMatch("mytest.log")))
			return false;
		
		//////////
		
		// test FilenameMatcherNameWildcard - match right
		FilenameMatcherNameWildcard fm2_3("*est", "log");
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_3.canSkipPotentialFile("mytest")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension", fm2_3.canSkipPotentialFile("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension and core filename", fm2_3.canSkipPotentialFile("tegst.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong core filename", fm2_3.canSkipPotentialFile("tegst.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong long core filename", fm2_3.canSkipPotentialFile("test_this_is_a_test_again.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong short core filename", fm2_3.canSkipPotentialFile("t.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and right core filename", fm2_3.canSkipPotentialFile("mytest.log")))
			return false;
		
		//
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_3.doesMatch("mytest")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension", fm2_3.doesMatch("mytest.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension and core filename", fm2_3.doesMatch("test.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong core filename", fm2_3.doesMatch("tegst.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong long core filename", fm2_3.doesMatch("test_this_is_a_test_again.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong short core filename", fm2_3.doesMatch("t.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and right core filename", fm2_3.doesMatch("mytest.log")))
			return false;
		
		/////////
		
		// test FilenameMatcherNameWildcard - match outer
		FilenameMatcherNameWildcard fm2_4("tes*ring", "log");
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_4.canSkipPotentialFile("teststring")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension", fm2_4.canSkipPotentialFile("teststring.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test wrong extension and core filename", fm2_4.canSkipPotentialFile("tststrin.txt")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong core filename", fm2_4.canSkipPotentialFile("rtsttstrin.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong core filename", fm2_4.canSkipPotentialFile("teststrin.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong core filename", fm2_4.canSkipPotentialFile("eststring.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong long core filename", fm2_4.canSkipPotentialFile("tefst_this_is_a_tefst_again.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and wrong short core filename", fm2_4.canSkipPotentialFile("e.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and right core filename", fm2_4.canSkipPotentialFile("teststring.log")))
			return false;

		//
		
		if (!CHECK_RETURN_FALSE("test possible directory", fm2_4.doesMatch("teststring")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension", fm2_4.doesMatch("teststring.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test wrong extension and core filename", fm2_4.doesMatch("tststrin.txt")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong core filename", fm2_4.doesMatch("tegst.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong long core filename", fm2_4.doesMatch("tegst_this_is_a_tegst_again.log")))
			return false;
		
		if (!CHECK_RETURN_FALSE("test right extension and wrong short core filename", fm2_4.doesMatch("t.log")))
			return false;
		
		if (!CHECK_RETURN_TRUE("test right extension and right core filename", fm2_4.doesMatch("teststring.log")))
			return false;
		
		return true;
	}
	
	
protected:
	bool CHECK_RETURN_TRUE(const std::string& description, bool retVal)
	{
		if (!retVal)
		{
			fprintf(stderr, "FAIL: %s\n", description.c_str());
			return false;
		}
		else
		{
			fprintf(stderr, "OK: %s\n", description.c_str());
			return true;
		}
	}

	bool CHECK_RETURN_FALSE(const std::string& description, bool retVal)
	{
		if (retVal)
		{
			fprintf(stderr, "FAIL: %s\n", description.c_str());
			return false;
		}
		else
		{
			fprintf(stderr, "OK: %s\n", description.c_str());
			return true;
		}
	}
	
};

#endif // TESTS_H

