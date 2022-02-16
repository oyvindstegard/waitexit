/*
 * Wait for N seconds or any input before exiting.
 * Prints a countdown in terminal while waiting to exit.
 * Author: Øyvind Stegard <oyvind@stegard.net>
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>

/* Get terminal width using ioctl */
static unsigned short term_width = 0;
static unsigned short get_terminal_width() {
  if (term_width == 0) {
    struct winsize sz;
    int result = ioctl(0, TIOCGWINSZ, &sz);
    if (result > -1) {
      term_width = sz.ws_col;
    }
  }
  return term_width;
}

/* Word wraps and aligns a prefix pluss descriptive text.
   For user friendly help text formatting in resizable terminal. */
#define FORMATTING_MAX_WIDTH 80
static void print_aligned(FILE* const out,
                          const char* first_line_prefix,
                          const char* text) {
  const unsigned short tw = get_terminal_width();
  const unsigned short formatting_width = tw > 0 && tw < FORMATTING_MAX_WIDTH ? tw : FORMATTING_MAX_WIDTH;

  const size_t prefix_len = strlen(first_line_prefix);
  fputs(first_line_prefix, out);

  size_t remain = strlen(text);
  char buf[1024 + remain];
  char* writeptr = buf;
  while (remain > 0) {
    while (isspace(*text)) ++text;
    int linemax = formatting_width - prefix_len;
    if (remain > linemax) {
      if (! isspace(text[linemax])) {
        // prefer to break line on word boundary
        while (linemax > 0 && ! isspace(text[linemax-1])) --linemax;
        if (linemax == 0) {
          // no word boundary found while back tracking, just break at end
          linemax = formatting_width - prefix_len;
        }
      }
      writeptr = stpncpy(writeptr, text, linemax);
      remain -= linemax;
      
      text += linemax;
      *(writeptr++) = '\n';
      for (int i=0; i<prefix_len; i++) *(writeptr++) = ' ';
    } else {
      // last line
      writeptr = stpncpy(writeptr, text, remain);
      strcpy(writeptr, "\n");
      break;
    }
  }
  fputs(buf, out);
}

static void print_usage(const char* self) {
  print_aligned(stderr, "", "Prints a countdown in terminal while waiting to exit.");
  print_aligned(stderr, "", "When timer reaches zero or any input occurs, the program exits.");
  print_aligned(stderr, "", "");

  char* bnbuf = strdup(self);
  char use_prefix[strlen(bnbuf)+100];
  sprintf(use_prefix, "Use: %s [opts] N, ", basename(bnbuf));
  free(bnbuf);
  print_aligned(stderr, use_prefix, "where N is number of seconds to wait.");

  print_aligned(stderr, "", "");
  print_aligned(stderr, "Options:", "");
  print_aligned(stderr, "-x      ", "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
  print_aligned(stderr, "-s      ", "Be completely silent, do not output anything while waiting. While testing this solution, there are necessary words lorem opsum dolor. Sit amet while waiting. For never ending. Great. 1 2 3 4 5. End of text.");
  
  print_aligned(stderr, "-m MSG  ", "Use a custom countdown message, which is a format string with single integer argument that is number of seconds left.");
  print_aligned(stderr, "-e CODE ", "Exit with status CODE.");
  print_aligned(stderr, "-h      ", "show this help.");
}

static const char* DEFAULT_MSG = "Hei there %s seconds left";

typedef struct {
  int countdown;
  unsigned int opts;
  unsigned char exitcode;
  char* msg;
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
