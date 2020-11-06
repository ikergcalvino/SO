#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

#define MAXTAM 4096

// implementación de la lista y funciones básicas
typedef struct
{
    char comando[MAXTAM];
} Element;

typedef struct
{
    Element *elements[MAXTAM];
    int size;
} List;

void CreateList(List *list)
{
    list->size = 0;
}

void InsertInList(List *list, char element[])
{
    list->elements[list->size] = (Element *)strdup(element);
    list->size++;
}

void ClearList(List *list)
{
    list->size = 0;
}

void PrintList(List *list, int N)
{
    int i;
    if (N <= list->size)
    {
        for (i = 0; i <= N; i++)
        {
            printf("%i: ", i);
            printf("%s", (list->elements[i])->comando);
        }
    }
    else
    {
        printf("%s\n", strerror(errno));
    }
}

// funciones auxiliares e implementación del shell

void process(char entrada[], List *lista, bool *bucle);

void prompt()
{
    printf("» ");
}

void authors(char *arg)
{
    if (arg == NULL)
    {
        printf("Íker García Calviño <iker.gcalvino>\n");
        printf("Juan Toiran Freire <juan.tfreire>\n");
    }
    else
    {
        if (strcmp(arg, "-l") == 0)
        {
            printf("iker.gcalvino\n");
            printf("juan.tfreire\n");
        }
        else if (strcmp(arg, "-n") == 0)
        {
            printf("Íker García Calviño\n");
            printf("Juan Toiran Freire\n");
        }
        else
        {
            printf("Error: argument not valid.\n");
        }
    }
}

void pid(char *arg)
{
    if (arg == NULL)
    {
        printf("PID of process: %i\n", getpid());
    }
    else
    {
        printf("Error: argument not valid.\n");
    }
}

void ppid(char *arg)
{
    if (arg == NULL)
    {
        printf("PID of parent process: %i\n", getppid());
    }
    else
    {
        printf("Error: argument not valid.\n");
    }
}

void pwd(char *arg)
{
    char cwd[MAXTAM];
    if (arg == NULL)
    {
        printf("%s\n", getcwd(cwd, MAXTAM));
    }
    else
    {
        printf("Error: argument not valid.\n");
    }
}

void cdir(char *arg)
{
    if (arg == NULL)
    {
        pwd(NULL);
    }
    else
    {
        if (chdir(arg) != 0)
        {
            printf("Error: %s.\n", strerror(errno));
        }
    }
}

void date(char *arg)
{
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    if (arg == NULL)
    {
        printf("%1.2d/%1.2d/%1.2d\n", local->tm_mday, local->tm_mon, 1900 + local->tm_year);
    }
    else
    {
        printf("Error: argument not valid.\n");
    }
}

void tiempo(char *arg)
{
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    if (arg == NULL)
    {
        printf("%d:%d:%d\n", local->tm_hour, local->tm_min, local->tm_sec);
    }
    else
    {
        printf("Error: argument not valid.\n");
    }
}

void historic(List *lista, char *arg)
{
    if (arg == NULL)
    {
        PrintList(lista, (lista->size));
    }
    else
    {
        if (strcmp(arg, "-c") == 0)
        {
            ClearList(lista);
        }
        else if (isdigit(arg[1]))
        {
            PrintList(lista, atoi(++arg));
        }
        else if (arg[1] == 'r' && isdigit(arg[2]))
        {
            arg++;
            printf("%s", strdup(lista->elements[atoi(++arg)]->comando));
            process(lista->elements[atoi(arg)]->comando, lista, false);
        }
        else
        {
            printf("Error: argument not valid.\n");
        }
    }
}

int trocear(char *cadena, char *trozos[])
{
    int i = 1;
    if ((trozos[0] = strtok(cadena, " \n\t")) == NULL)
    {
        return 0;
    }
    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL)
    {
        i++;
    }
    return i;
}

void process(char entrada[], List *lista, bool *bucle)
{
    char *trozos[15];
    int acceso = trocear(entrada, trozos);
    if (acceso > 0)
    {
        if (strcmp(trozos[0], "authors") == 0)
        {
            authors(trozos[1]);
        }
        else if (strcmp(trozos[0], "getpid") == 0)
        {
            pid(trozos[1]);
        }
        else if (strcmp(trozos[0], "getppid") == 0)
        {
            ppid(trozos[1]);
        }
        else if (strcmp(trozos[0], "pwd") == 0)
        {
            pwd(trozos[1]);
        }
        else if (strcmp(trozos[0], "chdir") == 0)
        {
            cdir(trozos[1]);
        }
        else if (strcmp(trozos[0], "date") == 0)
        {
            date(trozos[1]);
        }
        else if (strcmp(trozos[0], "time") == 0)
        {
            tiempo(trozos[1]);
        }
        else if (strcmp(trozos[0], "historic") == 0)
        {
            historic(lista, trozos[1]);
        }
        else if (strcmp(trozos[0], "quit") == 0 || strcmp(trozos[0], "end") == 0 || strcmp(trozos[0], "exit") == 0)
        {
            *bucle = true;
        }
        else
            printf("Error: command not found.\n");
    }
}

void reader(List *lista, char command[])
{
    fgets(command, MAXTAM, stdin);
    if ((int)command[0] != 10)
    {
        InsertInList(lista, command);
    }
}

int main(int argc, char const *argv[])
{
    List *lista = malloc(sizeof(List));
    char entrada[MAXTAM];
    CreateList(lista);
    bool bucle = false;
    while (!bucle)
    {
        prompt();
        reader(lista, entrada);
        process(entrada, lista, &bucle);
    }
    ClearList(lista);
    return 0;
}