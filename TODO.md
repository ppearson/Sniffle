Sniffle TODO:
-------------

* Multithread file finding (partial support in directory wildcard search mode currently)
* Multithread content searching
* Progressive content searching as files are found, rather than as a second step
* Outputting before content context lines (partial support in grep mode only currently)
* Outputting file size / file date along with filename
* More flexible and advanced file/directory pattern matching
* Use faster content-searching (Boyer-Moore or SIMD'd character matching),
  finding newlines after initial match
* More flexible output mode (built-in as opposed to piped to stdout), allowing outputting
  to multiple files based off file location subdirectory
* Support for more date formats in timestamp delta mode

