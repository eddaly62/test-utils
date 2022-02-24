README
This directory contains various utilities that can be used to test how terminal programs
respond to wscape codes defined in ANSI x3.4

termtest.c
-----------
This program is run from the command line of the terminal that you want to test.
It prepends a ESC character (27 dec) to the string that you type in.
When you press enter the packet gets processed by the terminal.

To build:
gcc -o termtest termtest.c

termtest
---------
This is termtest.c compiled with gcc on Ubuntu
To run:
./termtest

termtestsgr.c
--------------
This program will print numbers 0 to 107 on the console in a matrix.
Each number will represent a SGR display code.
Ref: Wikipedia
https://en.wikipedia.org/wiki/ANSI_escape_code

To build:
gcc -o termtestsgr termtestsgr.c

termtestsgr
------------
This is termtestsgr.c compiled with gcc on Ubuntu
To run:
./termtestsgr

