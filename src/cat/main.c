#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct st {
  bool b;
  bool e;
  bool n;
  bool s;
  bool t;
  bool v;
} st;

int parseArgs(int argc, char **argv, st *state, bool *flag);
int parsePaths(int startIndex, int n, char **argv, char ***paths);
void addFlag(char *flag, int type, st *state, bool *f);
bool printFiles(int argc, char **argv, st state);
void printFile(FILE *fp, st state);
void printLine(char *buf, size_t len);

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: cat [OPTION] [FILE]...\n");
    return -1;
  }

  bool flag = 0;
  char **paths = NULL;
  st state = {false};

  int pathsIndex, pathsCount;

  pathsIndex = parseArgs(argc, argv, &state, &flag);

  if (!flag) pathsCount = parsePaths(pathsIndex, argc, argv, &paths);

  if (!flag) {
    flag = printFiles(pathsCount, paths, state);
  }

  free(paths);

  return flag;
}

int parseArgs(int argc, char **argv, st *state, bool *flag) {
  int i;
  for (i = 1; i < argc && !(*flag); i++) {
    if (strlen(argv[i]) >= 2 && argv[i][0] == 45 && argv[i][1] != 45) {
      addFlag(argv[i], 1, state, flag);
    } else if (strlen(argv[i]) >= 2 && argv[i][0] == 45 && argv[i][1] == 45) {
      addFlag(argv[i], 2, state, flag);
    } else
      break;
  }
  return i;
}

int parsePaths(int startIndex, int n, char **args, char ***paths) {
  int pathCount = 0;
  for (int i = startIndex; i < n; i++) {
    ++pathCount;
    *paths = realloc(*paths, sizeof(char *) * pathCount);
    (*paths)[pathCount - 1] = args[i];
  }
  return pathCount;
}

void addFlag(char *arg, int type, st *state, bool *f) {
  if (type == 1) {
    for (int i = 1; (size_t)i < strlen(arg) && !(*f); i++) {
      switch (arg[i]) {
        case 'b':
          state->b = true;
          state->n = false;
          break;
        case 'e':
          state->e = true;
          state->v = true;
          break;
        case 'E':
          state->e = true;
          break;
        case 'n':
          if (!state->b) state->n = true;
          break;
        case 's':
          state->s = true;
          break;
        case 't':
          state->t = true;
          state->v = true;
          break;
        case 'T':
          state->t = true;
          break;
        case 'v':
          state->v = true;
          break;
        default:
          printf("cat: invalid option -- '%c'\n", arg[i]);
          *f = 1;
          break;
      }
    }
  }
  if (type == 2) {
    if (strcmp("--number-nonblank", arg) == 0) {
      state->b = true;
      state->n = false;
    }

    else if (strcmp("--number", arg) == 0) {
      if (!state->b) state->n = true;
    }

    else if (strcmp("--squeeze-blank", arg) == 0)
      state->s = true;

    else {
      printf("cat: unrecognized option '%s'\n", arg);
      *f = 1;
    }
  }
}

bool printFiles(int argc, char **argv, st state) {
  bool flag = 0;
  for (int i = 0; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");

    if (!fp) {
      printf("cat: %s: No such file or directory", argv[i]);
      flag = 1;
      continue;
    }

    printFile(fp, state);

    fclose(fp);
  }
  return flag;
}

void printFile(FILE *fp, st state) {
  char *buffer = NULL;
  size_t len = 0;
  ssize_t nread;
  bool secondSpace = false;
  int i = 0;
  while ((nread = getline(&buffer, &len, fp)) != -1) {
    i++;
    if (state.s) {
      if (nread > 1) secondSpace = false;

      if (secondSpace) {
        i--;
        continue;
      }

      if (nread <= 1 && buffer[0] == 10) secondSpace = true;
    }
    if (state.b) {
      if (nread <= 1 && buffer[0] == 10) {
        if (state.e) printf("%6c\t", 32);
        i--;
      } else
        printf("%6d\t", i);
    }
    if (state.n) {
      printf("%6d\t", i);
    }
    for (int j = 0; j < nread; j++) {
      if (state.e) {
        state.v = true;
        if (buffer[j] == 10) {
          printf("$\n");
          continue;
        }
      }
      if (state.v) {
        if (buffer[j] >= 0 && buffer[j] <= 31 && buffer[j] != 9 &&
            buffer[j] != 10) {
          printf("^%c", buffer[j] + 64);
          continue;
        } else if (buffer[j] == 127) {
          printf("^?");
          continue;
        }
      }
      if (state.t) {
        if (buffer[j] == '\t') {
          printf("^I");
          continue;
        }
      }
      putchar(buffer[j]);
    }
  }
  free(buffer);
}

void printLine(char *buf, size_t len) {
  for (int i = 0; (size_t)i < len; i++) {
    putchar(buf[i]);
  }
}
