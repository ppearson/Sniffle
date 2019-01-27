Sniffle changelog
=================

* Added short circuit option, which when enabled will make Sniffle abort processing the current file
  when it finds the specified string

Version 0.3
-----------

* Added count mode support, in order to count appearances of strings in files
* Added file size filtering support

Version 0.2
-----------

* Added basic parallel find functionality (multiple threads) when using directory-level wildcard
  searching
* Added basic file modified timestamp filter capability, to allow filtering files older/younger
  than a time delta from the current time
* Printouts of numeric matched file counts and content counts now have thousand separators
* Fixed bug which incorrectly skipped symlinks to files when preEmptiveSymlinkSkipping was
  enabled and the filename match extension was '*'
* Some minor internal refactoring/renaming to FilenameMatchers classes

Version 0.1
-----------

* Initial version
