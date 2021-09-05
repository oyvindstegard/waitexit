/*
 * Wait for N seconds or any input before exiting.
 * Prints a countdown in terminal while waiting to exit.
 * Auhtor: Øyvind Stegard <oyvind@stegard.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

#define DBG(m) fprintf(stderr, "dbg: %s\n", m)

static void print_usage(const char* self) {
  char* bnbuf = strdup(self);
  fprintf(stderr, "Prints a countdown in terminal while waiting to exit.\n");
  fprintf(stderr, "When timer reaches zero or any input occurs, the program exits.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use: %s [opts] N, where N is number of seconds to wait.\n", basename(bnbuf));
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "-s       be completely silent, do not output anything while waiting.\n");
  fprintf(stderr, "-e CODE  exit with status CODE\n");
  fprintf(stderr, "-h       show this help\n");
  free(bnbuf);
}

typedef struct {
  int countdown;
  unsigned int opts;
  unsigned char exitcode;
} Settings;

#define OPT_SILENT 0x1
#define OPT_HELP   0x2

/* Parse arguments and populate settings object, returns != 0 on success. */
static int parse_arguments(int argc, char** argv, Settings* settings) {
  int c;
  int val;
  settings->opts = 0;
  settings->countdown = -1;
  settings->exitcode = 0;

  opterr = 1;
  
  while ((c = getopt(argc, argv, "she:")) != -1) {
    switch(c) {
    case 's':
      settings->opts |= OPT_SILENT;
      break;
    case 'e':
      if (sscanf(optarg, "%i", &val) != 1) {
        fprintf(stderr, "Error: -e requires an integer argument: %s\n", optarg);
        return 0;
      }
      if (val < 0 || val > 255) {
        fprintf(stderr, "Error: -e requires integer argument between 0 and 255: %s\n", optarg);
        return 0;
      }
      settings->exitcode = val;
      break;
    case 'h':
      settings->opts |= OPT_HELP;
      break;
    case '?':
      return 0;
    }
  }

  if (optind < argc) {
    if (sscanf(argv[optind], "%i", &val) != 1 || val < 0) {
      fprintf(stderr, "Error: countdown must be a positive integer: %s\n", argv[optind]);
      return 0;
    }
    settings->countdown = val;
  }
  
  return 1;
}

/* Waits at most one second for a character to be read from stdin. */
static int wait_key_pressed() {
  fd_set read_stdin_fds;
  FD_ZERO(&read_stdin_fds);
  FD_SET(STDIN_FILENO, &read_stdin_fds);
  
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  int retval = select(fileno(stdin)+1, &read_stdin_fds, NULL, NULL, &tv);
  if (retval) {
    char devnull[1024];
    read(fileno(stdin), &devnull, sizeof(devnull));
    return retval;
  }

  return 0;
}

static struct termios default_term;

static void reset_termio() {
  tcsetattr(fileno(stdin), TCSANOW, &default_term);
}

static void init_termio() {
  // Unbuffered standard out
  setvbuf(stdout, NULL, _IONBF, 0);

  // Non-canonical non-echoing stdin if tty
  if (isatty(fileno(stdin))) {
    tcgetattr(fileno(stdin), &default_term);
    atexit(reset_termio);
  
    struct termios term;
    tcgetattr(fileno(stdin), &term);
  
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(fileno(stdin), TCSANOW, &term);
  }
}

int main(int argc, char ** argv) {

  Settings settings;
  if (!parse_arguments(argc, argv, &settings)) {
    return 1;
  }

  if (settings.opts & OPT_HELP) {
    print_usage(argv[0]);
    return 0;
  }

  if (settings.countdown < 0) {
    fprintf(stderr, "Error: number of seconds to wait must be specified.\n");
    return 1;
  }

  init_termio();

  int seconds = settings.countdown;

  int seconds_left = seconds;
  while (seconds_left > 0) {
    if (! (settings.opts & OPT_SILENT)) {
      fprintf(stdout, "Waiting for %i seconds, press any key to exit..", seconds_left);
    }
    if (wait_key_pressed()) {
      break;
    }
    
    seconds_left = seconds_left - 1;
    

    if (! (settings.opts & OPT_SILENT)) {
      fprintf(stdout, "\r\033[K");
    }
  }
  if (! (settings.opts & OPT_SILENT)) {
    fprintf(stdout, "\r\033[KExited with status %i after %i seconds.\n",
            settings.exitcode, seconds-seconds_left);
  }

  return settings.exitcode;
}
