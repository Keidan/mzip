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
#include "mzip_config.h"
#include <getopt.h>
#include <signal.h>
#include <tk/utils/string.h>
#include <tk/sys/syssig.h>
#include <tk/utils/z.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tk/sys/systools.h>

static z_t z = NULL;

static const struct option long_options[] = { 
    { "help"          , 0, NULL, 'h' },
    { "file"          , 1, NULL, 'f' },
    { "password"      , 1, NULL, 'p' },
    { "create"        , 0, NULL, 'c' },
    { "extract"       , 0, NULL, 'x' },
    { "level"         , 1, NULL, 'l' },
    { "append"        , 0, NULL, 'a' },
    { "exclude-path"  , 0, NULL, 'e' },
    { "directory"     , 1, NULL, 'd' },
    { NULL            , 0, NULL,  0  } 
};

static void mzip_signals(int s);
static void mzip_cleanup(void);

/**
 * Affichage du 'usage'.
 * @param err Code passe a exit.
 */
void usage(int err) {
  fprintf(stdout, "usage: mzip [options]\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  fprintf(stdout, "\t--file, -f: ZIP file.\n");
  fprintf(stdout, "\t--password, -p: ZIP file password.\n");
  fprintf(stdout, "\t----------------------\n");
  fprintf(stdout, "\t--create, -c: Action mode create.\n");
  fprintf(stdout, "\t--level, -l: The compression level 0 <= l <= 9 (default: store).\n");
  fprintf(stdout, "\t\t0=store only, 1=faster, 9=better.\n");
  fprintf(stdout, "\t--append, -a: Append into an existing zip (default: no).\n");
  fprintf(stdout, "\t--exclude-path, -e: Exclude the file path (default: no).\n");
  fprintf(stdout, "\t--directory, -d: Directory to be compress.\n");
  fprintf(stdout, "\t----------------------\n");
  fprintf(stdout, "\t--extract, -x: Action mode extract.\n");
  exit(err);
}

void mzip_uncompress_callback(z_t z, struct zentry_s entry) {
  file_name_t dir;
  file_dirname(entry.name, dir);
  if(!file_is_dir(dir)) file_mkdirs(dir);
  if(!entry.isdir) {
    char ssize[SYSTOOLS_MAX_SSIZE];
    systools_size_to_string(entry.info.uncompressed_size, ssize);
    printf("Create file: %s (%s)\n", entry.name, ssize);
    FILE* f = fopen(entry.name, "w+");
    if(!f) return;
    if(entry.info.uncompressed_size)
      fwrite(entry.content, 1, entry.info.uncompressed_size, f);
    fclose(f);
  }
}

int main(int argc, char** argv) {
  _Bool create = 0, extract = 0, append, exclude_path;
  z_clevel_et level = Z_C_STORE;
  z_file_t filename, password, directory;

  syssig_init(log_init_cast_user("mzip", LOG_PID|LOG_CONS|LOG_PERROR), mzip_cleanup);
  syssig_add_signal(SIGINT, mzip_signals);
  syssig_add_signal(SIGTERM, mzip_signals);

  fprintf(stdout, "MyZIP is a FREE software v%d.%d.\nCopyright 2011-2013 By kei\nLicense GPL.\n", MZIP_VERSION_MAJOR, MZIP_VERSION_MINOR);

  bzero(filename, sizeof(z_file_t));
  bzero(password, sizeof(z_file_t));

  int opt;
  while ((opt = getopt_long(argc, argv, "hf:cxp:l:aed:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(0); break;
      case 'f': /* file */
	strncpy(filename, optarg, sizeof(z_file_t));
	break;
      case 'p': /* password */
	strncpy(password, optarg, sizeof(z_file_t));
	break;
      case 'd': /* directory */
	strncpy(directory, optarg, sizeof(z_file_t));
	break;
      case 'l': /* level */
	level = string_parse_int(optarg, 0);
	if(level < 0) level = Z_C_STORE;
	else if(level > Z_C_BETTER) level = Z_C_BETTER;
	break;
      case 'c': /* create */
	create = 1;
	break;
      case 'x': /* extract */
	extract = 1;
	break;
      case 'e': /* exclude-path */
	exclude_path = 1;
	break;
      case 'a': /* append */
	append = 1;
	break;
      default: /* '?' */
	logger(LOG_ERR, "Unknown option '%c'\n", opt);
	usage(EXIT_FAILURE);
	break;
    }
  }
  if(!strlen(filename)) {
    logger(LOG_ERR, "Invalid zip file name!\n");
    usage(EXIT_FAILURE);
  }
  if(!extract && !create) {
    logger(LOG_ERR, "Please set an action mode!\n");
    usage(EXIT_FAILURE);
  } else if(extract && create) {
    logger(LOG_ERR, "Exctract and create are sets!\n");
    usage(EXIT_FAILURE);
  }
  z = z_new();

  if(extract) {
    if(z_open(z, filename)) {
      logger(LOG_ERR, "Unable to open the zip fie %s\n", filename);
      return EXIT_FAILURE;
    }
    if(z_uncompress(z, password, mzip_uncompress_callback)) {
      logger(LOG_ERR, "Unable to extract the zip fie %s\n", filename);
      return EXIT_FAILURE;
    }
  } else {
    fifo_t files = fifo_new(1);
    if(file_list_dir(directory, files)) {
      logger(LOG_ERR, "Unable to get the file list of the directory %s\n", directory);
      return EXIT_FAILURE;
    }
    if(z_compress(z, filename, password, level, append, exclude_path, files)) {
      fifo_delete(files);
      logger(LOG_ERR, "Unable to create the zip file %s\n", filename);
      return EXIT_FAILURE;
    }
    fifo_delete(files);
  }

  return EXIT_SUCCESS;
}


static void mzip_signals(int s) {
  if(s == SIGINT)
    printf("\n"); // for ^C
  exit(0); /* call atexit */
}

static void mzip_cleanup(void) {
  if(z) z_delete(z), z = NULL;
}
