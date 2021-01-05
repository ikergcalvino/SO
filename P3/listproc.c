#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "listproc.h"

/****************************** SENALES ******************************/

struct SEN
{
    char *nombre;
    int senal;
};

static struct SEN sigstrnum[] = {
    "HUP", SIGHUP,
    "INT", SIGINT,
    "QUIT", SIGQUIT,
    "ILL", SIGILL,
    "TRAP", SIGTRAP,
    "ABRT", SIGABRT,
    "IOT", SIGIOT,
    "BUS", SIGBUS,
    "FPE", SIGFPE,
    "KILL", SIGKILL,
    "USR1", SIGUSR1,
    "SEGV", SIGSEGV,
    "USR2", SIGUSR2,
    "PIPE", SIGPIPE,
    "ALRM", SIGALRM,
    "TERM", SIGTERM,
    "CHLD", SIGCHLD,
    "CONT", SIGCONT,
    "STOP", SIGSTOP,
    "TSTP", SIGTSTP,
    "TTIN", SIGTTIN,
    "TTOU", SIGTTOU,
    "URG", SIGURG,
    "XCPU", SIGXCPU,
    "XFSZ", SIGXFSZ,
    "VTALRM", SIGVTALRM,
    "PROF", SIGPROF,
    "WINCH", SIGWINCH,
    "IO", SIGIO,
    "SYS", SIGSYS,
/*senales que no hay en todas partes*/
#ifdef SIGPOLL
    "POLL", SIGPOLL,
#endif
#ifdef SIGPWR
    "PWR", SIGPWR,
#endif
#ifdef SIGEMT
    "EMT", SIGEMT,
#endif
#ifdef SIGINFO
    "INFO", SIGINFO,
#endif
#ifdef SIGSTKFLT
    "STKFLT", SIGSTKFLT,
#endif
#ifdef SIGCLD
    "CLD", SIGCLD,
#endif
#ifdef SIGLOST
    "LOST", SIGLOST,
#endif
#ifdef SIGCANCEL
    "CANCEL", SIGCANCEL,
#endif
#ifdef SIGTHAW
    "THAW", SIGTHAW,
#endif
#ifdef SIGFREEZE
    "FREEZE", SIGFREEZE,
#endif
#ifdef SIGLWP
    "LWP", SIGLWP,
#endif
#ifdef SIGWAITING
    "WAITING", SIGWAITING,
#endif
    NULL, -1,
}; /*fin array sigstrnum */

int Senal(char *sen) /*devuel el numero de senial a partir del nombre*/
{
    int i;
    for (i = 0; sigstrnum[i].nombre != NULL; i++)
        if (!strcmp(sen, sigstrnum[i].nombre))
            return sigstrnum[i].senal;
    return -1;
}

char *NombreSenal(int sen) /*devuelve el nombre senal a partir de la senal*/
{                          /* para sitios donde no hay sig2str*/
    int i;
    for (i = 0; sigstrnum[i].nombre != NULL; i++)
        if (sen == sigstrnum[i].senal)
            return sigstrnum[i].nombre;
    return ("SIGUNKNOWN");
}

static struct node *createnode()
{
    struct node *tmp = malloc(sizeof(struct node));
    if (tmp == NULL)
    {
        printf("memoria agotada\n");
        exit(EXIT_FAILURE);
    }
    return tmp;
}

lProc createlProc()
{
    struct node *list = createnode();
    list->sig = NULL;
    return list;
}

int emptylist(lProc list) { return (list->sig == NULL); }

void insertProc(dProc dp, pProc p)
{
    struct node *tmp = createnode();
    tmp->dp = dp;
    tmp->sig = p->sig;
    p->sig = tmp;
}

pProc findProc(lProc list, pid_t pid)
{
    struct node *p = list->sig;
    while (p != NULL && p->dp.pid != pid)
        p = p->sig;
    return p;
}

void deleteProc(lProc list, pProc p)
{
    struct node *tmp;
    tmp = list;
    while (tmp->sig != p && tmp->sig != NULL)
        tmp = tmp->sig;
    tmp->sig = p->sig;
    free(p);
}

pProc firstProc(lProc list) { return list->sig; }

pProc nextProc(pProc p) { return p->sig; }

int endlProc(pProc p) { return (p == NULL); }

dProc element(pProc p) { return p->dp; }

void modElement(pProc p)
{
    int state;
    if (getpriority(PRIO_PROCESS, p->dp.pid) != -1)
        p->dp.priority = getpriority(PRIO_PROCESS, p->dp.pid);
    if (waitpid(p->dp.pid, &state, WNOHANG | WUNTRACED | WCONTINUED) == p->dp.pid)
    {
        if (WIFEXITED(state))
        {
            strcpy(p->dp.state, "Terminated Normally");
            return;
        }
        if (WIFCONTINUED(state))
        {
            strcpy(p->dp.state, "Running");
            return;
        }
        if (WIFSTOPPED(state))
        {
            strcpy(p->dp.state, "Stopped");
            p->dp.signal = NombreSenal(WIFSTOPPED(state));
            return;
        }
        if (WTERMSIG(state))
        {
            strcpy(p->dp.state, "Terminated by signal");
            p->dp.signal = NombreSenal(WTERMSIG(state));
            return;
        }
    }
}