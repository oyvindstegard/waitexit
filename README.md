# waitexit

## Build (on Linux-ish with gcc available)

    make
    
## Installation

Copy binary to wherever you like.
    
## Usage

    $ ./waitexit -h
    Prints a countdown in terminal while waiting to exit.
    When timer reaches zero or any input occurs, the program exits.

    Use: waitexit [opts] N, where N is number of seconds to wait.

    Options:
    -s       be completely silent, do not output anything while waiting.
    -e CODE  exit with status CODE
    -h       show this help
    
## Purpose

May be a useful command for scripts which run in pop-up terminal windows, and
for which you would like to keep the terminal open for some amount of time at
exit, before automatically closing the window.
  
Other than that, maybe as an example of how to write a "press any key" CLI event
handler in C or using the `getopt` argument parser.
