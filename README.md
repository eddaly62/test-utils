README.md

This directory contains various utilities that can be used to test how terminal programs
respond to escape codes defined in ANSI x3.4

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


