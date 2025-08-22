/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File         : minishell.c
   Compiler/System : gcc/linux
********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20           /* max number of command tokens */
#define NL 100          /* input buffer size */
char            line[NL];   /* command input buffer */

/* minimal job table for background processes */
struct job { pid_t pid; int active; char cmd[NL]; } jobs[64];

/* record a background job */
static void add_job(pid_t pid, char *argv[]) {
  for (int j = 0; j < 64; ++j) {
    if (!jobs[j].active) {
      jobs[j].active = 1;
      jobs[j].pid = pid;
      jobs[j].cmd[0] = '\0';
      for (int k = 0; argv[k]; ++k) {             /* build "cmd args" */
        strncat(jobs[j].cmd, argv[k], NL-1 - strlen(jobs[j].cmd));
        if (argv[k+1]) strncat(jobs[j].cmd, " ", NL-1 - strlen(jobs[j].cmd));
      }
      return;
    }
  }
}

/* print Done for a finished background job and clear it */
static void report_done(pid_t pid) {
  for (int j = 0; j < 64; ++j) {
    if (jobs[j].active && jobs[j].pid == pid) {
      printf("[#]+ Done                 %s\n", jobs[j].cmd);
      fflush(stdout);
      jobs[j].active = 0;
      break;
    }
  }
}

/* block and reap all remaining children before exiting (minimal) */
static void drain_all_children(void) {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, 0)) > 0) {   /* block until none left */
    report_done(pid);                              /* only bg jobs print Done */
  }
}

/* SIGCHLD: reap only known background PIDs to avoid stealing foreground */
void sigchld_handler(int sig) {
  (void)sig;
  int status;
  for (int j = 0; j < 64; ++j) {
    if (jobs[j].active) {
      pid_t r = waitpid(jobs[j].pid, &status, WNOHANG);
      if (r == jobs[j].pid) {
        report_done(r);                            
      }
    }
  }
}

/*
  shell prompt
 */
void prompt(void)
{
  // ## REMOVE THIS 'fprintf' STATEMENT BEFORE SUBMISSION
  // fprintf(stdout, "\n msh> ");
  // fflush(stdout);
}


/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argk, char *argv[], char *envp[])
{
  int            frkRtnVal;     /* value returned by fork sys call */
  char           *v[NV];        /* array of pointers to command line tokens */
  char           *sep = (char*)" \t\n";  /* command line token separators    */
  int            i;             /* parse index */

  /* install SIGCHLD handler for background reaping */
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) perror("sigaction");

  /* prompt for and process one command line at a time  */
  while (1) {         /* do Forever */
    prompt();
    if (fgets(line, NL, stdin) == NULL) { /* read a line */
      if (feof(stdin)) {      /* non-zero on EOF  */
        drain_all_children();                 /* <<< ensure bg completion is reported */
        exit(0);
      }
      perror("fgets");
      continue;
    }
    fflush(stdin);

    // This if() required for gradescope
    if (feof(stdin)) {    /* non-zero on EOF  */
      drain_all_children();                   /*  ensure bg completion is reported */
      exit(0);
    }
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000'){
      continue;     /* to prompt */
    }

    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL){
        break;
      }
    }
    /* assert i is number of tokens + 1 */
    if (!v[0]) continue;

    /* built-in: exit */
    if (strcmp(v[0], "exit") == 0) {
      drain_all_children();                   
      exit(0);
    }

    /* built-in: cd (cd with no arg -> $HOME) */
    if (strcmp(v[0], "cd") == 0) {
      const char *target = v[1] ? v[1] : getenv("HOME");
      if (!target) target = ".";
      if (chdir(target) == -1) perror("chdir");
      continue;
    }

    /* detect background '&' and remove it from argv */
    int background = 0;
    if (i > 1 && v[i-1] && strcmp(v[i-1], "&") == 0) {
      background = 1;
      v[i-1] = NULL;
    }

    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork()) {
      case -1:      /* fork returns error to parent process */
      {
        perror("fork");
        break;
      }
      case 0:       /* code executed only by child process */
      {
        execvp(v[0], v);
        perror("execvp");     /* exec returns only on error */
        _exit(127);           /* terminate child correctly */
      }
      default:      /* code executed only by parent process */
      {
        if (background) {
          add_job(frkRtnVal, v);             /* remember background cmd */
          printf("[#] %d\n", frkRtnVal);     /* print background pid (as required) */
          fflush(stdout);
          /* do not wait here */
        } else {
          if (waitpid(frkRtnVal, NULL, 0) == -1) perror("waitpid"); /* foreground wait */
        }
        break;
      }
    }       
  }        
}          
