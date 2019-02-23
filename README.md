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
matching multiple search strings within files, and more functionality will be added in the future.

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


File filtering:
---------------

Filtering can be done as an additional condition on the initial files found, filtering for file modified date and the file size.

Filter to search for files younger than two weeks old:

    sniffle -ff-md y14d grep "Error 101" "/path/to/logs/*/program/*prog*.log"

Filter to search for files older than 5 hours old:

    sniffle -ff-md o5h grep "Error 101" "/path/to/logs/*/program/*prog*.log"

Filter to search for files smaller than 12 MB in size:

    sniffle -ff-s s12m grep "Error 101" "/path/to/logs/*/program/*prog*.log"

Filter to search for files bigger than 1 GB in size:

    sniffle -ff-s b1g grep "Error 101" "/path/to/logs/*/program/*prog*.log"

