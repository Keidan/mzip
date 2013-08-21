/**
 *******************************************************************************
 * @file mzip.c
 * @author Keidan
 * @date 03/02/2013
 * @par Project
 * unzip
 *
 * @par Copyright
 * Copyright 2011-2013 Keidan, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */
#include <getopt.h>
#include <signal.h>
#include <tk/text/string.h>
#include <tk/sys/ssig.h>
#include <tk/sys/z.h>
#include <sys/stat.h>
#include <sys/types.h>


static z_t z = NULL;

static const struct option long_options[] = { 
    { "help"   , 0, NULL, 'h' },
    { "input"  , 1, NULL, 'o' },
    { "create" , 0, NULL, 'c' },
    { "extract", 0, NULL, 'x' },
    { NULL     , 0, NULL,  0  } 
};

static void mzip_signals(int s);
static void mzip_cleanup(void);

/**
 * Affichage du 'usage'.
 * @param err Code passe a exit.
 */
void usage(int err) {
  fprintf(stdout, "usage: mzip --input [zip] [action]\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  fprintf(stdout, "\t--input, -i: ZIP file.\n");
  fprintf(stdout, "\t--create, -c: Action mode create.\n");
  exit(err);
}

void mzip_file_content(z_t z, struct zentry_s entry) {
  if(entry.isdir) {
    printf("Create directory: %s\n", entry.name);
    mkdir(entry.name, 0777);
  } else {
    printf("Create file: %s\n", entry.name);
    FILE* f = fopen(entry.name, "w+");
    fwrite(entry.content, 1, entry.info.uncompressed_size, f);
    fclose(f);
  }
}

int main(int argc, char** argv) {
  _Bool create = 0;
  _Bool extract = 0;
  char filename[FILENAME_MAX];

  ssig_init(log_init_cast_user("mzip", LOG_PID|LOG_CONS|LOG_PERROR), mzip_cleanup);
  ssig_add_signal(SIGINT, mzip_signals);
  ssig_add_signal(SIGTERM, mzip_signals);

  fprintf(stdout, "Mzip is a FREE software.\nCopyright 2013-2013 By kei\nLicense GPL.\n\n");

  bzero(filename, FILENAME_MAX);

  int opt;
  while ((opt = getopt_long(argc, argv, "hi:cx", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(0); break;
      case 'i': /* input */
	strncpy(filename, optarg, FILENAME_MAX);
	break;
      case 'c':
	create = 1;
	break;
      case 'x':
	extract = 1;
	break;
      default: /* '?' */
	logger(LOG_ERR, "Unknown option '%c'\n", opt);
	usage(EXIT_FAILURE);
	break;
    }
  }
  if(!extract && !create) {
    logger(LOG_ERR, "Please set an action mode!\n");
    usage(EXIT_FAILURE);
  } else if(!extract && !create) {
    logger(LOG_ERR, "Exctract and create are sets!\n");
    usage(EXIT_FAILURE);
  }

  z = z_open(filename);
  if(z_get_contents(z, mzip_file_content))
    logger(LOG_ERR, "Unable to get the zip fie content\n");

  return EXIT_SUCCESS;
}


static void mzip_signals(int s) {
  if(s == SIGINT)
    printf("\n"); // for ^C
  exit(0); /* call atexit */
}

static void mzip_cleanup(void) {
  if(z) z_close(z), z = NULL;
}
