/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/
#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define NV  20     /* max number of command tokens */
#define NL  100    /* input buffer size           */
#define MAX_JOBS 128

static char line[NL];      /* command input buffer */
static volatile sig_atomic_t sigchld_flag = 0;  /* set in SIGCHLD handler */

typedef struct {
    int   id;              /* job counter, starts from 1 */
    pid_t pid;             /* child's PID */
    char  cmd[NL];         /* command string to print when done */
    int   active;          /* 1=running, 0=free */
} Job;

static Job jobs[MAX_JOBS];
static int next_job_id = 1;

// prompt
static void prompt(void)
{
    /* Keep prompt simple; only print when reading from a TTY */
    if (isatty(STDIN_FILENO) == 1) {
        fprintf(stdout, "\n msh> ");
        fflush(stdout);
    }
}

/* --------------------------------------------------------------- */
/* SIGCHLD handler: keep it async-signal-safe, set a flag only     */
/* --------------------------------------------------------------- */
static void on_sigchld(int signo)
{
    (void)signo;
    sigchld_flag = 1;
}

// Job helpers  
static int add_job(pid_t pid, const char *cmd)
{
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (!jobs[i].active) {
            jobs[i].active = 1;
            jobs[i].pid = pid;
            jobs[i].id  = next_job_id++;
            snprintf(jobs[i].cmd, sizeof(jobs[i].cmd), "%s", cmd);
            return jobs[i].id;
        }
    }
    return -1;
}

static Job* find_job_by_pid(pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (jobs[i].active && jobs[i].pid == pid) return &jobs[i];
    }
    return NULL;
}

static void remove_job(Job *j)
{
    if (j) j->active = 0;
}

/* --------------------------------------------------------------- */
/* Reap finished background children and print "[#]+ Done  <cmd>"  */
/* --------------------------------------------------------------- */
static void reap_children(void)
{
    int   status;
    pid_t pid;

    /* Reap all finished children without blocking */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        Job *j = find_job_by_pid(pid);
        if (j) {
            /* Match the assignment's formatting */
            printf("[#%d]+ Done\t\t%s\n", j->id, j->cmd);
            fflush(stdout);
            remove_job(j);
        }
    }
    if (pid == -1 && errno != ECHILD) {
        perror("waitpid");
    }
    sigchld_flag = 0;
}

// ------------------------------------------------------------
// Entry function (main function)
// ------------------------------------------------------------
int main(int argk, char *argv[], char *envp[])
{
    (void)argk;
    (void)argv;
    (void)envp; /* unused in this simple shell */

    /* Install SIGCHLD handler (reap in main loop) */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* restart interrupted syscalls */
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    /* Tokenization setup */
    char *v[NV];
    const char *sep = " \t\n";

    while (1) {
        /* If any background jobs finished since last loop, report now */
        if (sigchld_flag) reap_children();

        prompt();

        if (fgets(line, NL, stdin) == NULL) {
            if (ferror(stdin)) perror("fgets");
            /* EOF or read error: exit shell gracefully */
            break;
        }
        fflush(stdin); /* not needed on POSIX, kept to match original */

        /* Gradescope requires this EOF test too */
        if (feof(stdin)) {
            exit(0);
        }

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') {
            continue; /* comment/empty line */
        }

        /* Parse tokens */
        v[0] = strtok(line, sep);
        int i;
        for (i = 1; i < NV; i++) {
            v[i] = strtok(NULL, sep);
            if (v[i] == NULL) break;
        }
        /* i is number of tokens + 1 (as in original comment) */
        if (!v[0]) continue;

        /* Detect background flag if last token is "&" */
        int background = 0;
        if (i > 1 && v[i-1] && strcmp(v[i-1], "&") == 0) {
            background = 1;
            v[i-1] = NULL; /* remove '&' from argv for execvp */
            --i;
        }

        /* Built-in: exit */
        if (strcmp(v[0], "exit") == 0) {
            /* Try to reap any remaining children (non-blocking) */
            reap_children();
            break;
        }

        /* Built-in: cd */
        if (strcmp(v[0], "cd") == 0) {
            const char *target = (v[1] ? v[1] : getenv("HOME"));
            if (!target) target = ".";
            if (chdir(target) == -1) {
                perror("chdir");
            }
            if (sigchld_flag) reap_children();
            continue;
        }

        /* Reconstruct a command string for job completion message */
        char cmd_buf[NL] = {0};
        {
            size_t left = sizeof(cmd_buf) - 1;
            for (int k = 0; v[k] && left > 0; ++k) {
                size_t need = strnlen(v[k], left);
                strncat(cmd_buf, v[k], left);
                left = (left > need) ? (left - need) : 0;
                if (v[k+1] && left > 1) {
                    strncat(cmd_buf, " ", left);
                    --left;
                }
            }
        }

        /* Fork & exec */
        pid_t frkRtnVal = fork();
        if (frkRtnVal == -1) {           /* fork error in parent */
            perror("fork");
            continue;
        }

        if (frkRtnVal == 0) {            /* child */
            /* Restore default SIGINT so Ctrl+C can terminate foreground jobs */
            struct sigaction dfl;
            memset(&dfl, 0, sizeof(dfl));
            dfl.sa_handler = SIG_DFL;
            sigemptyset(&dfl.sa_mask);
            if (sigaction(SIGINT, &dfl, NULL) == -1) {
                perror("sigaction(SIGINT, SIG_DFL)");
                _exit(127);
            }

            execvp(v[0], v);
            /* If execvp returns, it failed */
            perror("execvp");
            _exit(127); /* ensure child terminates */
        } else {                           /* parent */
            if (background) {
                int jid = add_job(frkRtnVal, cmd_buf);
                if (jid < 0) {
                    fprintf(stderr, "jobs table full\n");
                }
                /* Immediate "[#] PID" as required */
                printf("[#%d] %d\n", jid, frkRtnVal);
                fflush(stdout);
                /* do NOT wait; go on to next command */
            } else {
                /* Foreground: wait for the child to finish */
                int status = 0;
                if (waitpid(frkRtnVal, &status, 0) == -1) {
                    perror("waitpid");
                }
            }

            /* After launching, report any background jobs that just finished */
            if (sigchld_flag) reap_children();
        }
    }

    /* Final reap to avoid zombies */
    reap_children();
    return 0;
}