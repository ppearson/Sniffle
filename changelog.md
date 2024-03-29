Sniffle changelog
=================

Version 0.6.3
-------------

* Added support for wildcard file matching with two wildcards in the core filename in limited combinations (more work needed to more fully support all combinations).

Version 0.6.2
-------------

* Fixed issue which meant that finding exact filenames in the current working directory didn't work.

Version 0.6.1
-------------

* Moved match argument validation to happen before searching for files.

Version 0.6
-----------

* Added support for tsdelta file processing, which will detect gaps greater than the specified time period in
  timestamps at the beginning of each line in files.
* Minor tweaks and optimisations.

Version 0.5
-----------

* Implemented more comprehensive filename pattern matching.
* Implemented more comprehensive and accurate filename preemptive skipping.
* Added fileReadBufferSize option, which allows overriding of default (now 32KB, previously was 8KB) read buffer size.
* #ifdeffed out some debug code that was doing extra opendir()/closedir() calls that wasn't really necessary.

Version 0.4
-----------

* Added short circuit option, which when enabled will make Sniffle abort processing the current file
  when it finds the specified string.
* Optimised file finding algorithm to check filename before doing a stat() for any find filters, so as
  to avoid needlessly doing stat() calls on files which were never going to match anyway.
* Added support for outputting content lines before the found line (-B or with -C for both before and after)
  in grep mode (support for within other modes to follow later).
* Added special-casing to file find functionality to allow specifying a single file (effectively short-circuits
  the find process).

Version 0.3
-----------

* Added count mode support, in order to count appearances of strings in files.
* Added file size filtering support.

Version 0.2
-----------

* Added basic parallel find functionality (multiple threads) when using directory-level wildcard
  searching.
* Added basic file modified timestamp filter capability, to allow filtering files older/younger
  than a time delta from the current time.
* Printouts of numeric matched file counts and content counts now have thousand separators.
* Fixed bug which incorrectly skipped symlinks to files when preEmptiveSymlinkSkipping was
  enabled and the filename match extension was '*'.
* Some minor internal refactoring/renaming to FilenameMatchers classes.

Version 0.1
-----------

* Initial version
