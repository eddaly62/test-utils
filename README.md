README.md

This directory contains various utilities

Prerequisites
--------------
If you do not have a build enviroment installed, you can install one by doing the following.
For Linux:
sudo apt update
sudo apt install build-essential

To test the install:
gcc --version
Should see version number


termtest.c
-----------
This program is run from the command line of the terminal that you want to test.
It prepends a ESC character (27 dec) to the string that you type in.
When you press enter the packet gets processed by the terminal.

To build:
gcc -o termtest termtest.c

To run:
./termtest


termtestsgr.c
--------------
This program will print numbers 0 to 107 on the console in a matrix.
Each number will represent a SGR display code.
Ref: Wikipedia
https://en.wikipedia.org/wiki/ANSI_escape_code
There is a picture at the bottom of this wikipedia page that shows what the screen
will look like.

To build:
gcc -o termtestsgr termtestsgr.c

To run:
./termtestsgr


fbinfo.c
----------
This program queries the display's frame buffer and displays the attributes to the console.

To build:
gcc fbinfo.c -o fbinfo

To run:
sudo ./fbinfo [path to frame buffer]
sudo ./fbinfo /dev/fp0

Note: to avoid the sudo thing...you can add yourself to the video group.

sudo adduser dell video

adds the user dell to video group
The frame buffers belon to the video group
You only need to do this once.

pattern.c
----------
This program creates several threads to search a table of regular expressions
to find a callback function to execute for the matching regular expression.

The input string is typed in via the keyboard and the number of threads created
to search the table of regular expressions is an input arguement to the program.
The range is 2 to 7.

The results are written to the display along with the measured search time.
If a match is found it will run the callback function.

This implemenation creates the threads once and keeps them alive for the duration of
the program. It uses barriers to provide synchronization between the threads.
The data exchange between the threads is accomplished with arrays, every thread gets
its own storage location in the arrays, avoiding the use of mutexes which
would slow the processing down.
Once the search is completed the search threads are waited and the main thread
goes and processes the results with no conflicts.

Plan to try variation of this that may increase processing speed.

To build:
gcc pattern.c -o pattern -lpthread

To run:
./pattern [2-7]
Where:
    [2-7] is the number of threads to spin up to search the regular expression callback table
For example:
    ./pattern 2
launches 2 threads

Enter the simulated data via the keyboard. Press enter to process


pattern2.c
--------------
This program creates several threads to search a table of regular expressions
to find a callback function to execute for the matching regular expression.

The input string is typed in via the keyboard and the number of threads created
to search the table of regular expressions is an input arguement to the program.
The range is 2 to 7.

The results are written to the display along with the measured search time.
If a match is found it will run the callback function.

This implemenation has a function that creates the threads for the search and once the 
search completes the threads are released. 

The function uses one barriers to provide synchronization between the threads.
The data exchange between the threads is accomplished with arrays, every thread gets
its own storage location in the arrays, avoiding the use of mutexes which
would slow the processing down. Once the search is completed the threads end and 
the results are collected with no conflicts.

To build:
gcc pattern2.c -o pattern2 -lpthread

To run:
./pattern2 [2-7]
Where:
    [2-7] is the number of threads to spin up to search the regular expression callback table
For example:
    ./pattern2 2
launches 2 threads

Enter the simulated data via the keyboard. Press enter to process


pattern3q.c
------------
This program creates several threads to search a table of regular expressions
to find a callback function to execute for the matching regular expression.

The input string is typed in via the keyboard and the number of threads created
to search the table of regular expressions is an input arguement to the program.
The range is 2 to 7.

The results are written to the display along with the measured search time.
If a match is found it will run the callback function.

This implemenation has a function that creates the threads for the search and once the 
search completes the threads are released. 

The function uses one barriers to provide synchronization between the threads.
The data exchange between the threads is accomplished with arrays, every thread gets
its own storage location in the arrays, avoiding the use of mutexes which
would slow the processing down. Once the search is completed the threads end and 
the results are collected with no conflicts.

This version changes the return and input ares of the dap_pattern_find function and
introduces a queue functionality.

To build:
gcc pattern3q.c -o pattern3q -lpthread

To run:
./pattern3q [2-7]
Where:
    [2-7] is the number of threads to spin up to search the regular expression callback table
For example:
    ./pattern3q 2
launches 2 threads

Enter the simulated data via the keyboard. Press enter to process