/**
 *******************************************************************************
 * @file unzip.c
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
#include <tk/sys/uz.h>
#include <sys/stat.h>
#include <sys/types.h>


static uz_t uz = NULL;

static const struct option long_options[] = { 
    { "help"   , 0, NULL, 'h' },
    { "input"  , 1, NULL, 'o' },
    { NULL     , 0, NULL,  0  } 
};

static void unzip_sig_int(sig_t s);
static void unzip_cleanup(void);

/**
 * Affichage du 'usage'.
 * @param err Code passe a exit.
 */
void usage(int err) {
  fprintf(stdout, "usage: unzip --input [zip] options\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  fprintf(stdout, "\t--input, -i: ZIP file.\n");
  exit(err);
}

void unzip_file_content(uz_t uz, struct uzentry_s entry) {
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
  char filename[FILENAME_MAX];
  fprintf(stdout, "Unzip is a FREE software.\nCopyright 2013-2013 By kei\nLicense GPL.\n\n");
  /* pour fermer proprement sur le kill */
  atexit(unzip_cleanup);
  signal(SIGINT, (__sighandler_t)unzip_sig_int);

  bzero(filename, FILENAME_MAX);

  int opt;
  while ((opt = getopt_long(argc, argv, "hi:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(0); break;
      case 'i': /* input */
	strncpy(filename, optarg, FILENAME_MAX);
	break;
      default: /* '?' */
	fprintf(stderr, "Unknown option '%c'\n", opt);
	usage(EXIT_FAILURE);
	break;
    }
  }

  uz = uz_open(filename);
  if(uz_get_contents(uz, unzip_file_content))
    fprintf(stderr, "Unable to get the zip fie content\n");

  return EXIT_SUCCESS;
}


static void unzip_sig_int(sig_t s) {
  printf("\n"); /* saute le ^C qui s'affiche dans la console... */
  exit(0); /* call atexit */
}

static void unzip_cleanup(void) {
  if(uz) uz_close(uz), uz = NULL;
}
