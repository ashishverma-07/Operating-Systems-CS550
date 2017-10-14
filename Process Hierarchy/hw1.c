#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void run(const char *param, int H, int C, int I, char *indent) {
  int i;
  pid_t *children = (pid_t *)malloc(sizeof(pid_t) * C);
  if (children == NULL) {
    printf("%sCould not allocate memory\n", indent);
  } else {
    /* construct the parameters for the child process */
    char h[32];
    char c[32];
    char idt[32];
    sprintf(h, "%d", H);
    sprintf(c, "%d", C);
    sprintf(idt, "%d", I);

    /* create the children processes */
    for (i = 0; i < C; i++) {
      children[i] = fork();
      if (children[i] == -1) {
        printf("%s(%u): Could not create child\n", indent, getpid());
      } else if (children[i] == 0) {
        /* use exec function to run the program recursively */
        if (execl(param, param, h, c, idt, NULL) < 0) {
          printf("%s(%u): Could not execute %s\n", indent, getpid(), param);
          exit(-1);
        }
      }
    }
    /* wait until all children terminate */
    for (i = 0; i < C; i++) {
      int status;
      /* make sure this child exited indeed (not stopped) */
      while (waitpid(children[i], &status, 0) != children[i] ||
          !WIFEXITED(status));
    }

    free(children);
  }
}

int main(int argc, char *argv[]) {
  const char *param = argv[0]; /* get parameters */
  int H = atoi(argv[1]);
  int C = atoi(argv[2]);
  char indent[128] = "";
  int I = 0; /* number of indent */
  int i;
  if (argc == 4) { /* construct the indent */
    I = atoi(argv[3]);
    for (i = 0; i < I; i++) {
      sprintf(&indent[2 * i], "  ");
    } 
  }

  if (H == 0) {
    printf("Could not execute.\n");
    exit(-1);
  }

  /* print the information of current process */
  pid_t pid = getpid();
  pid_t ppid = getppid();
  if (pid == -1 || ppid == -1) {
    printf("%sCould not get the process id\n", indent);
  }
  printf("%s(%u): Process starting\n", indent, pid);
  printf("%s(%u): Parent's id = (%u)\n", indent, pid, ppid);
  printf("%s(%u): Height in the tree = (%d)\n", indent, pid, H);
  printf("%s(%u): Creating (%d) children at height (%d)\n",
      indent, pid, C, H - 1);

  if (H > 1) {
    run(param, H - 1, C, I + 1, indent); /* create children with height H - 1*/
  }

  printf("%s(%u): Terminating at height (%d).\n", indent, pid, H);
  return 0;
}