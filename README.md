Sniffle
-------

Created by Peter Pearson in 2018.

Sniffle is a program designed to allow easy and flexible searching of text in log files,
with an emphasis on performing the search over busy NFS networks where there are many symlinks
involved in the directory/file structure.

It is *not* designed as a complete grep / ack / ag replacement for general file searches
(although it can do that with certain by-design limitations).

It can perform file searches, grepping of content of files, matching multiple search strings
within files, and more functionality will be added in the future.

Find:
-----

Return a list of all files matching a filename pattern in specified directory location(s).
This find functionality is part of all other modes of operation below.

sniffle find "/path/to/logs/*/program/*.log"

Grep:
-----

Output matches of content in found files that contain content.

sniffle grep "Error 101" "/path/to/logs/*/program/*.log"

Match:
------

Output matches of content in found files that contain multiple content matches (on different lines).

Match any:

sniffle match "Program 1.1.|[Error] " "/path/to/logs/*/program/*prog*.log"

Match all (in order):

sniffle match "Program 1.1.&[Error] 101" "/path/to/logs/*/program/*prog*.log"


