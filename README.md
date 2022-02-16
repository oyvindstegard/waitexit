
# waitexit

## Build (on Linux-ish with gcc available)

    make
    
## Installation

Copy binary to wherever you like.
    
## Usage

    $ ./waitexit -h
    Prints a countdown in terminal while waiting to exit. When timer reaches zero or
    any input occurs, the program exits.

    Use: waitexit [opts] N, where N is number of seconds to wait.

    Options:
    -m MSG  Use a custom countdown message template, where '%S' is replaced by 
            number of seconds left.
    -e CODE Exit with status CODE.
    -f      Exit with status 0 if user presses a key within the timeout, otherwise 
            exit with non-zero code.
    -z      Suppress printing of wait time and status code on exit.
    -s      Be completely silent, do not output anything while waiting or on exit.
    -h      Show this help.
    
## Purpose

May be a useful command for scripts which run in pop-up terminal windows, and
for which you would like to keep the terminal open for some amount of time at
exit, before automatically closing the window.
  
Other than that, maybe as an example of how to write a "press any key" CLI event
handler in C or using the `getopt` argument parser.
