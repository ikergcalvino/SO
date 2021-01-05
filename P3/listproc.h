typedef struct dProc
{
    pid_t pid;
    int priority;
    char *signal;
    char line[20];
    char state[20];
    time_t timestamp;
} dProc;

struct node
{
    dProc dp;
    struct node *sig;
};

typedef struct node *pProc;
typedef struct node *lProc;

int Senal(char *sen);
char *NombreSenal(int sen);
static struct node *createnode();
lProc createlProc();
int emptylist(lProc list);
void insertProc(dProc dp, pProc p);
pProc findProc(lProc list, pid_t pid);
void deleteProc(lProc list, pProc p);
pProc firstProc(lProc list);
pProc nextProc(pProc p);
int endlProc(pProc p);
dProc element(pProc p);
void modElement(pProc p);