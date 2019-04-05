Sniffle
-------

Created by Peter Pearson in 2018.

Sniffle is a program designed to allow fast, easy and flexible searching of text in log files,
with a heavy emphasis on performing the search over busy NFS networks where there are many symlinks
involved in the directory/file structure, and where the number of resultant files to match content
in can number the hundreds of thousands and where directory/file operations / syscalls can have
a significant overhead to finding files.

As such, it has the ability to semi-intelligently skip doing expensive stat calls on unknown file
types (which can happen a lot on large NFS networks) based off the apparent filename of the
directory entry, and has defaults (which can be configured with a config file) which make sense
for this workflow compared to other grep/search tools (i.e. following symlinks is enabled by default).

It is *not* designed as a complete grep / ack / ag replacement for general file searches on local file
systems, although it can do that to a degree with certain by-design limitations: however, it is likely
that these other tools will be more efficient for searching local (non-network) filesystems.

Currently it does not use the most optimum algorithm for searching content (currently uses
linear line-by-line search instead of something like Boyer-Moore) - in the future this will
be improved.

It can perform file searches, grepping of content of files, counting occurrences of strings, 
matching multiple search strings within files, detecting timestamp deltas within log files,
and more functionality will be added in the future.

Find:
-----

Return a list of all files matching a filename pattern in specified directory location(s).
This find functionality is part of all other modes of operation below.

    sniffle find "/path/to/logs/*/program/*.log"

Grep:
-----

Output matches of content in found files that contain content.

    sniffle grep "Error 101" "/path/to/logs/*/program/*.log"

Count:
------

Output counts of the occurrences of the searched-for string within each file, where count > 0.

    sniffle count "[Warning 552]" "/path/to/logs/*/program/*.log"


Match:
------

Output matches of content in found files that contain multiple content matches (on different lines).

Match any:

    sniffle match "Program 1.1.|[Error] " "/path/to/logs/*/program/*prog*.log"

Match all (in order):

    sniffle match "Program 1.1.&[Error] 101" "/path/to/logs/*/program/*prog*.log"


Timestamp Deltas:
---------------

Outputs pairs of lines which have timestamps of which the time delta between them is greater than or equal to the specified amount.
I.e. to look for delays / pauses between timestamps on subsequent lines in a log. Due to algorithm optimisations, there's a limit on this
functionality working correctly with the limitation of timestamp delta being less than a month: it's possible the functionality won't
work correctly with larger timestamp deltas than that.

The timestamp string currently must be at the beginning of each line, and be in the approximate format: YYYY-MM-DD HH:MM:SS

Separators between each value *can* be different (i.e. spaces instead of dashes), but the length must be consistent and the values
must be in the expected character position.

Leading and trailing surrounding characters separating the timestamp from the rest of the log line output are supported
(defaults to "[%ts%]"), and the characters can be modified and removed via the 'logTimestampFormat' config option.

Example log lines which are supported (although 'logTimestampFormat' will need to be modified to support the different types):
Default - with 'logTimestampFormat' = '[%ts%]':

    [2019-03-28 08:23:33] Event1
    [2019-03-28 08:25:33] Event2

With 'logTimestampFormat' = '(%ts%)':

    (2019-03-28 08:23:33) Event1
    (2019-03-28 08:25:33) Event2

With 'logTimestampFormat' = '%ts%':

    2019-03-28 08:23:33 Event1
    2019-03-28 08:25:33 Event2

Example usages:

Find timestamp deltas of greater than or equal to 10 minutes:

    sniffle tsdelta 10 "/path/to/logs/*/program/*prog*.log"

Find timestamp deltas of greater than or equal to 4 hours:

    sniffle tsdelta 4h "/path/to/logs/*/program/*prog*.log"


File filtering:
-------------

Filtering can be done as an additional condition on the initial files found, filtering for file modified date or the file size.

Filter to search for files younger than two weeks old:

    sniffle -ff-md y14d grep "Error 101" "/path/to/logs/*/program/*prog*.log"

Filter to search for files older than 5 hours old:

    sniffle -ff-md o5h grep "Error 101" "/path/to/logs/*/program/*prog*.log"

Filter to search for files smaller than 12 MB in size:

    sniffle -ff-s s12m grep "Error 101" "/path/to/logs/*/program/*prog*.log"

Filter to search for files bigger than 1 GB in size:

    sniffle -ff-s b1g grep "Error 101" "/path/to/logs/*/program/*prog*.log"


Short circuiting
--------------

All file processing operations support the ability to short circuit processing of files when a certain string is found.
When this string is found, processing of the particular file will be stopped.

This can significantly speed up processing of files in the situation where the content in the files you're looking for
is within a particular stage of log output, i.e. if you know the output you're looking for can only occur in
log lines before a certain unique log line is output, this functionality can be used to skip uselessly processing the rest
of each file by specifying that unique string as the short circuit option.

It can be specified with the -sc "string to look for" option before the processing mode, i.e.:

    sniffle -sc "Building BVH..." grep "[Error] Degenerate geo found" "/path/to/logs/*/program/*prog*.log"

