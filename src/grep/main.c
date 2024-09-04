#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

typedef struct state {
  regmatch_t match;
  int matchCount;
  int lenE;
  int lenF;
  char **e;
  char **f;
  bool multFiles;
  int i;
  bool v;
  bool c;
  bool l;
  bool n;
  bool h;
  bool s;
  bool o;
} st;

bool parser(int argc, char **argv, st *state);
int countFiles(int argc, char **argv);
bool processFile(int argc, char **argv, st *state);
int findTemp(FILE *fp, st *state, char *path);
bool printLine(int val, char *buffer, st *state, char *path, int line);

int main(int argc, char **argv) {
  bool flag = 1;
  st state = {0};

  if (argc < 3) {
    printf("Usage: grep [OPTIONS] template [FILENAME]\n");
    flag = 0;
  }

  if (flag) {
    flag = parser(argc, argv, &state);
  }

  if (flag) {
    if ((argc - optind) > 1) state.multFiles = true;
    flag = processFile(argc, argv, &state);
  }

  for (int i = 0; i < state.lenE; i++) free(state.e[i]);
  free(state.e);

  for (int i = 0; i < state.lenF; i++) free(state.f[i]);
  free(state.f);

  return flag ? 0 : 1;
}

bool parser(int argc, char **argv, st *state) {
  int res = 0;
  bool flag = 1;

  while ((res = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (res) {
      case 'e':
        state->e = realloc(state->e, ++state->lenE * sizeof(char *));
        state->e[state->lenE - 1] = malloc(strlen(optarg) * sizeof(char) + 1);
        strcpy(state->e[0], optarg);
        break;
      case 'i':
        state->i = REG_ICASE;
        break;
      case 'v':
        state->v = true;
        break;
      case 'c':
        state->c = true;
        break;
      case 'l':
        state->l = true;
        break;
      case 'n':
        state->n = true;
        break;
      case 'h':
        state->h = true;
        break;
      case 's':
        state->s = true;
        break;
      case 'f':
        state->f = realloc(state->f, ++state->lenF * sizeof(char *));
        state->f[state->lenF - 1] = malloc(strlen(optarg) * sizeof(char) + 1);
        strcpy(state->f[state->lenF - 1], optarg);
        break;
      case 'o':
        state->o = true;
        break;
      case '?':
      default:
        flag = 0;
        break;
    }
  }

  if (state->lenE == 0 && state->lenF == 0) {
    state->e = realloc(state->e, ++state->lenE * sizeof(char *));
    state->e[0] = malloc(strlen(argv[optind]) + 1);
    strcpy(state->e[0], argv[optind]);
    optind++;
  }

  return flag;
}

bool processFile(int argc, char **argv, st *state) {
  bool flag = 1;

  for (int i = optind; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");
    if (!fp) {
      if (!state->s) printf("grep: %s: No such file or directory\n", argv[i]);
      flag = 0;
      continue;
    }
    findTemp(fp, state, argv[i]);
    fclose(fp);
  }

  return flag;
}

int findTemp(FILE *fp, st *state, char *path) {
  regex_t reg;
  size_t len = 0;
  ssize_t nread;
  char *buffer = NULL;
  int val;
  int line = 1;

  while ((nread = getline(&buffer, &len, fp)) != -1) {
    bool printed = false;

    for (int i = 0; i < state->lenF && !printed; i++) {
      FILE *tempf = fopen(state->f[i], "r");

      if (!tempf) {
        if (!state->s)
          printf("grep: %s: No such file or directory\n", state->f[i]);
        exit(1);
      }

      size_t tempLen = 0;
      ssize_t tempRead;
      char *temp = NULL;
      while ((tempRead = getline(&temp, &tempLen, tempf)) != -1 && !printed) {
        char *res = temp;
        regcomp(&reg, res, REG_EXTENDED | state->i);
        val = regexec(&reg, buffer, 1, &state->match, 0);

        printed = printLine(val, buffer, state, path, line);
        regfree(&reg);
      }
      fclose(tempf);
      free(temp);
    }

    for (int i = 0; i < state->lenE && !printed; i++) {
      regcomp(&reg, state->e[i], REG_EXTENDED | state->i);
      val = regexec(&reg, buffer, 1, &state->match, 0);

      printed = printLine(val, buffer, state, path, line);
      regfree(&reg);
    }

    line++;
  }

  if (state->c) {
    if (state->multFiles && !state->h) printf("%s:", path);
    printf("%d\n", state->matchCount);
  }
  if (state->l && state->matchCount > 0) {
    printf("%s\n", path);
  }

  state->matchCount = 0;

  free(buffer);

  return 1;
}

bool printLine(int val, char *buffer, st *state, char *path, int line) {
  bool printed = false;
  if ((val == 0 && !state->v) || (val != 0 && state->v)) {
    if (state->l) state->matchCount = 1;

    if (state->c && !state->l) state->matchCount++;

    if (!state->c && !state->l) {
      if (state->multFiles && !state->h) printf("%s:", path);
      if (state->n) printf("%d:", line);

      if (state->o && val == 0) {
        for (int i = state->match.rm_so; i < state->match.rm_eo; i++)
          printf("%c", buffer[i]);
      } else
        printf("%s", buffer);

      if (buffer[strlen(buffer) - 1] != '\n') putchar('\n');
    }
    printed = true;
  }

  return printed;
}
