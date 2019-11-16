// created by: WestleyR
// email: westleyr@nym.hush.com
// https://github.com/WestleyR/txtlinter
// date: Nov 16, 2019
// version-1.0.0
//
// The Clear BSD License
//
// Copyright (c) 2019 WestleyR
// All rights reserved.
//
// This software is licensed under a Clear BSD License.
//

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>

#define SCRIPT_VERSION "v1.0.0-beta-2, Nov 16, 2019"

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

// skip_chars are chars that the linter will skip if a line
// starts with one of these chars.
static const char skip_chars[] = {' ', '-', '#', '*', '\0'};

void print_version() {
  printf("%s\n", SCRIPT_VERSION);
}

void print_commit_hash() {
  printf("%s\n", COMMIT_HASH);
}

void print_usage(const char* script_name) {
  printf("Usage:\n");
  printf("  %s [option] <path> <file>\n", script_name);
  printf("\n");
  printf("Options:\n");
  printf("  -w, --write    rewrite file(s)\n");
  printf("  -c, --check    only check if file(s) need linting (comming soon)\n");
  printf("  -t, --type     specify a file type (default .rst)\n");
  printf("  -p, --path     specify a path to lint (comming soon)\n");
  printf("  -f, --file     specify a file to lint (comming soon)\n");
  printf("  -h, --help     print this help menu\n");
  printf("  -C, --commit   print the github commit hash\n");
  printf("  -V, --version  print the version\n");
  printf("\n");
  printf("Source code: https://github.com/WestleyR/txtlinter\n");
}

void add_char_to_string(char* s, char c) {
  int len = strlen(s);
  s[len] = c;
  s[len+1] = '\0';
}

int next_word(const char* line, int index) {
  int ret = 0;

  for (int i = index; i < strlen(line); i++) {
    if (line[i] == ' ' || line[i] == '\n') {
      break;
    } else {
      ret++;
    }
  }

  return(ret);
}

int file_extension(char* string, char* extension) {
  string = strrchr(string, '.');
  if (string != NULL) {
    if (strcmp(string, extension) == 0) {
      return(0);
    }
  }

  return(1);
}

int lint_file(const char* file) {
  FILE* fp = fopen(file, "r");
  if (fp == NULL) {
    perror(file);
    return(1);
  }

  char newfile[100];
  newfile[0] = '\0';

  strcpy(newfile, file);
  strcat(newfile, ".writting");

  FILE* newfp = fopen(newfile, "w");
  if (newfp == NULL) {
    perror(newfile);
    return(1);
  }

  char line[2560];
  char new_line[2560];
  new_line[0] = '\0';
  int line_len = 0;
  int line_max = 0;
  int last_newline = 0;
  int in_line = 0;

  while (fgets(line, sizeof line, fp) != NULL) {
    if (strlen(line) > 80 || in_line == 1) {
      in_line = 1;
      int skip_line = 0;
      for (int i = 0; i < strlen(skip_chars); i++) {
        if (skip_chars[i] == line[0]) {
          skip_line = 1;
        }
      }

      if (skip_line == 0) {
        for (int c_line = 0; c_line < strlen(line); c_line++) {
          line_len++;
          line_max++;

          if (line[c_line] == ' ') {
            if ((line_len + next_word(line, line_max)) > 80 - 10) {
              add_char_to_string(new_line, ' ');
              add_char_to_string(new_line, '\n');

              line_len = 0;
            }
          }

          if (line_len != 0 && line[0] != ' ') {
            if (line[c_line] != '\n') {
              add_char_to_string(new_line, line[c_line]);
            } else if (line_max < 80) {
              add_char_to_string(new_line, '\n');
              add_char_to_string(new_line, '\n');
              line_len = 0;
              in_line = 0;
            }
          }
        }

        fputs(new_line, newfp);
        new_line[0] = '\0';
        line_max = 0;
      } else {
        fputs(line, newfp);
      }
    } else {
      new_line[0] = '\0';

      for (int i = 0; i < strlen(line); i++) {
        if (line[i] != '\n') {
          add_char_to_string(new_line, line[i]);
        } else if (last_newline == 1) {
          add_char_to_string(new_line, '\n');
        } else if (strlen(line) <= 80) {
          add_char_to_string(new_line, '\n');
        }
      }

      fputs(new_line, newfp);
      new_line[0] = '\0';
    }

    if (line[0] == '\n') {
      last_newline = 1;
    } else {
      last_newline = 0;
    }
  }

  fclose(fp);
  fclose(newfp);

  printf("I: copying files...\n");

  fp = fopen(file, "w");
  if (fp == NULL) {
    perror(file);
    return(1);
  }

  newfp = fopen(newfile, "r");
  if (newfp == NULL) {
    perror(newfile);
    return(1);
  }

  char ch;

  while ((ch = fgetc(newfp)) != EOF) {
    fputc(ch, fp);
  }

  fclose(fp);
  fclose(newfp);

  if (remove(newfile) != 0) {
    perror(newfile);
    fprintf(stderr, "Failed to remove tmp file\n");
    return(1);
  }

  return(0);
}

int run_lint(char *path, char* file_type) {
  DIR* dr;
  struct dirent *de;

  int files_exists = 0;

  dr = opendir("./");
  while ((de = readdir(dr)) != NULL) {
    if ((*de->d_name == '.') || (strcmp(de->d_name, "..") == 0)) {
      continue;
    }

    if (file_extension(de->d_name, file_type) == 0) {
      files_exists = 1;
      printf("I: linting file: %s\n", de->d_name);
      lint_file(de->d_name);
    } else {
      printf("I: skipping file: %s\n", de->d_name);
    }
  }

  if (!files_exists) {
    fprintf(stderr, "No files in \"%s\" that have: %s extension\n", path, file_type);
    return(1);
  }

  return(0);
}

int main(int argc, char** argv) {
  int opt = 0;

  // Options:
  int write = 0;
  int check = 0;
  char file_type[10];

  file_type[0] = '\0';
  strcpy(file_type, ".rst");

  static struct option long_options[] = {
    {"write", no_argument, 0, 'w'},
    {"check", no_argument, 0, 'c'},
    {"type", required_argument, 0, 't'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"commit", no_argument, 0, 'C'},
    {NULL, 0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "t:wchVC", long_options, 0)) != -1) {
    switch (opt) {
      case 'w':
        write = 1;
        break;

      case 'c':
        check = 1;
        break;

      case 't':
        file_type[0] = '\0';
        strcpy(file_type, optarg);
        break;

      case 'h':
        print_usage(argv[0]);
        return(0);

      case 'V':
        print_version();
        return(0);

      case 'C':
        print_commit_hash();
        return(0);

      default:
        return(22);
    }
  }

  if (write) {
    if (run_lint("./", file_type) != 0) {
      return(1);
    }
  } else if (check) {
    //check_line("./");
    printf("Not yet...\n");
    return(1);
  } else {
    fprintf(stderr, "--write or --check not used, nothing to do...\n");
    return(1);
  }

  return(0);
}

// vim: tabstop=2 shiftwidth=2 expandtab
