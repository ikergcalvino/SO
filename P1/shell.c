#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXTAM 4096

/*
 * Íker García Calviño
 * Juan Toirán Freire 
 */

// funciones aux p1

char LetraTF(mode_t m)
{
    switch (m & S_IFMT)
    { /*and bit a bit con los bits de formato,0170000*/
    case S_IFSOCK:
        return 's'; /*socket*/
    case S_IFLNK:
        return 'l'; /*symbolic link*/
    case S_IFREG:
        return '-'; /*fichero normal*/
    case S_IFBLK:
        return 'b'; /*block device*/
    case S_IFDIR:
        return 'd'; /*directorio*/
    case S_IFCHR:
        return 'c'; /*char  device*/
    case S_IFIFO:
        return 'p'; /*pipe*/
    default:
        return '?'; /*desconocido, no deberia aparecer*/
    }
}

char *ConvierteModo(mode_t m, char *permisos)
{
    strcpy(permisos, "---------- ");
    permisos[0] = LetraTF(m);
    if (m & S_IRUSR)
        permisos[1] = 'r'; /*propietario*/
    if (m & S_IWUSR)
        permisos[2] = 'w';
    if (m & S_IXUSR)
        permisos[3] = 'x';
    if (m & S_IRGRP)
        permisos[4] = 'r'; /*grupo*/
    if (m & S_IWGRP)
        permisos[5] = 'w';
    if (m & S_IXGRP)
        permisos[6] = 'x';
    if (m & S_IROTH)
        permisos[7] = 'r'; /*resto*/
    if (m & S_IWOTH)
        permisos[8] = 'w';
    if (m & S_IXOTH)
        permisos[9] = 'x';
    if (m & S_ISUID)
        permisos[3] = 's'; /*setuid, setgid y stickybit*/
    if (m & S_ISGID)
        permisos[6] = 's';
    if (m & S_ISVTX)
        permisos[9] = 't';
    return permisos;
}

char *ConvierteModo2(mode_t m)
{
    static char permisos[12];
    strcpy(permisos, "---------- ");
    permisos[0] = LetraTF(m);
    if (m & S_IRUSR)
        permisos[1] = 'r'; /*propietario*/
    if (m & S_IWUSR)
        permisos[2] = 'w';
    if (m & S_IXUSR)
        permisos[3] = 'x';
    if (m & S_IRGRP)
        permisos[4] = 'r'; /*grupo*/
    if (m & S_IWGRP)
        permisos[5] = 'w';
    if (m & S_IXGRP)
        permisos[6] = 'x';
    if (m & S_IROTH)
        permisos[7] = 'r'; /*resto*/
    if (m & S_IWOTH)
        permisos[8] = 'w';
    if (m & S_IXOTH)
        permisos[9] = 'x';
    if (m & S_ISUID)
        permisos[3] = 's'; /*setuid, setgid y stickybit*/
    if (m & S_ISGID)
        permisos[6] = 's';
    if (m & S_ISVTX)
        permisos[9] = 't';
    return (permisos);
}

char *ConvierteModo3(mode_t m)
{
    char *permisos;
    permisos = (char *)malloc(12);
    strcpy(permisos, "---------- ");
    permisos[0] = LetraTF(m);
    if (m & S_IRUSR)
        permisos[1] = 'r'; /*propietario*/
    if (m & S_IWUSR)
        permisos[2] = 'w';
    if (m & S_IXUSR)
        permisos[3] = 'x';
    if (m & S_IRGRP)
        permisos[4] = 'r'; /*grupo*/
    if (m & S_IWGRP)
        permisos[5] = 'w';
    if (m & S_IXGRP)
        permisos[6] = 'x';
    if (m & S_IROTH)
        permisos[7] = 'r'; /*resto*/
    if (m & S_IWOTH)
        permisos[8] = 'w';
    if (m & S_IXOTH)
        permisos[9] = 'x';
    if (m & S_ISUID)
        permisos[3] = 's'; /*setuid, setgid y stickybit*/
    if (m & S_ISGID)
        permisos[6] = 's';
    if (m & S_ISVTX)
        permisos[9] = 't';
    return (permisos);
}

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
    if (N < list->size)
    {
        for (i = 0; i <= N; i++)
        {
            printf("%i: ", i);
            printf("%s", (list->elements[i])->comando);
        }
    }
    else if (N == list->size)
    {
        for (i = 0; i < N; i++)
        {
            printf("%i: ", i);
            printf("%s", (list->elements[i])->comando);
        }
    }
    else
    {
        printf("Error: position %i not valid.\n", N);
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
    else if (strcmp(arg, "-l") == 0)
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
    else if (chdir(arg) != 0)
    {
        printf("Error: %s.\n", strerror(errno));
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
    else if (strcmp(arg, "-c") == 0)
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
        if (atoi(++arg) < lista->size)
        {
            printf("%s", strdup(lista->elements[atoi(arg)]->comando));
            process(lista->elements[atoi(arg)]->comando, lista, false);
        }
        else
        {
            printf("Error: command at position %i not valid.\n", atoi(arg));
        }
    }
    else
    {
        printf("Error: argument not valid.\n");
    }
}

void info(char *file, bool tam)
{
    char link[MAXTAM];
    char fecha[MAXTAM];
    struct stat st, stl;
    struct tm *time = localtime(&st.st_atime);
    struct group *group = getgrgid(st.st_uid);
    struct passwd *user = getpwuid(st.st_uid);
    char *permission = ConvierteModo2(st.st_mode);

    if (stat(file, &st) != -1)
    {
        if (tam)
        {
            strftime(fecha, sizeof(fecha), "%b %d %H:%M", time);
            printf("%s\t%6ld\t%s\t%s\t%s\t%8ld\t(%2ld)", fecha, st.st_ino, user->pw_name, group->gr_name, permission, st.st_size, st.st_nlink);
            lstat(file, &stl);
            if (S_ISLNK(stl.st_mode))
            {
                readlink(file, link, MAXTAM);
                printf("\t%s -> %s\n", file, link);
            }
            else
            {
                printf("\t%s\n", file);
            }
        }
        else
        {
            printf("%ld\t%s \n", st.st_size, file);
        }
    }
    else
    {
        printf("%s\n", strerror(errno));
    }
}

void list_dir(char *direct, bool hid, bool lon)
{
    DIR *directory;
    struct dirent *act;

    directory = opendir(direct);

    if (directory != NULL)
    {
        while ((act = readdir(directory)) != NULL)
        {
            if (!hid)
            {
                info(act->d_name, lon);
            }
            else if ((strcmp(act->d_name, ".") != 0) && (strcmp(act->d_name, "..") != 0))
            {
                info(act->d_name, lon);
            }
        }
        closedir(directory);
    }
    else
    {
        printf("%s\n", strerror(errno));
    }
}

void create(char *arg[])
{
    if (arg[1] == NULL)
    {
        list_dir(".", false, false);
    }
    else if (strcmp(arg[1], "-dir") == 0)
    {
        if (mkdir(arg[2], S_IRWXU | S_IRWXG) == -1)
        {
            printf("%s\n", strerror(errno));
        }
    }
    else if (open(arg[1], O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
    {
        printf("%s\n", strerror(errno));
    }
}

void del_rec(char *direct)
{
    DIR *directory;
    struct stat st;
    struct dirent *act;

    directory = opendir(direct);

    if (directory != NULL)
    {
        if (chdir(direct) != 0)
        {
            printf("Error: %s.\n", strerror(errno));
        }
        while ((act = readdir(directory)) != NULL)
        {
            if ((strcmp(act->d_name, ".") != 0) && (strcmp(act->d_name, "..") != 0))
            {
                if (stat(act->d_name, &st) == -1)
                {
                    printf("%s\n", strerror(errno));
                }
                else if (LetraTF(st.st_mode) == 'd')
                {
                    if (unlink(act->d_name) == -1)
                    {
                        printf("%s\n", strerror(errno));
                    }
                }
                else if (LetraTF(st.st_mode) == '-')
                {
                    if (remove(act->d_name) == -1)
                    {
                        printf("%s\n", strerror(errno));
                    }
                }
                del_rec(act->d_name);
            }
        }
        chdir("..");
    }
    else
    {
        printf("%s\n", strerror(errno));
        return;
    }
    closedir(directory);
    if (rmdir(direct) == -1)
    {
        printf("%s\n", strerror(errno));
    }
}

void del(char *arg[])
{
    int i;
    char directorio[MAXTAM];

    getcwd(directorio, MAXTAM);

    if (arg[1] == NULL)
    {
        list_dir(".", false, false);
    }
    else if (strcmp(arg[1], "-rec") == 0)
    {
        for (i = 1; arg[i] != NULL; i++)
        {
            del_rec(arg[i]);
            if (chdir(directorio) != 0)
            {
                printf("Error: %s.\n", strerror(errno));
            }
        }
    }
    else
    {
        for (i = 1; arg[i] != NULL; i++)
        {
            if (remove(arg[i]) == -1)
            {
                printf("%s\n", strerror(errno));
            }
        }
    }
}

void list_rec(char *direct, bool hid, bool lon)
{
    DIR *directory;
    struct stat st;
    struct dirent *act;

    list_dir(".", hid, lon);
    directory = opendir(direct);

    if (directory != NULL)
    {
        if (chdir(direct) == 0)
        {
            while ((act = readdir(directory)) != NULL)
            {
                stat(act->d_name, &st);
                if ((LetraTF(st.st_mode) == 'd') && (strcmp(act->d_name, ".") != 0) && (strcmp(act->d_name, "..") != 0))
                {
                    printf("\n");
                    if (chdir(act->d_name) != 0)
                    {
                        printf("Error: %s.\n", strerror(errno));
                    }
                    list_rec(act->d_name, hid, lon);
                    chdir("..");
                }
            }
            closedir(directory);
        }
        else
        {
            printf("Error: %s.\n", strerror(errno));
        }
    }
}

void list(char *arg[])
{
    int i;
    int flags = 1;
    bool l = false, d = false, h = false, r = false;

    for (i = 1; arg[i] != NULL; i++)
    {
        if (strcmp(arg[i], "-long") == 0)
        {
            l = true;
            flags++;
        }
        else if (strcmp(arg[i], "-dir") == 0)
        {
            d = true;
            flags++;
        }
        else if (strcmp(arg[i], "-hid") == 0)
        {
            h = true;
            flags++;
        }
        else if (strcmp(arg[i], "-rec") == 0)
        {
            r = true;
            flags++;
        }
        else
        {
            break;
        }
    }

    if (arg[flags] != NULL)
    {
        for (i = flags; arg[i] != NULL; i++)
        {
            if (d)
            {
                if (r)
                {
                    if (chdir(arg[i]) != 0)
                    {
                        list_rec(".", h, l);
                    }
                    else
                    {
                        list_rec(".", h, l);
                        chdir("..");
                    }
                }
                else
                {
                    if (chdir(arg[i]) != 0)
                    {
                        printf("Error: %s.\n", strerror(errno));
                    }
                    else
                    {
                        list_dir(".", h, l);
                        chdir("..");
                    }
                }
            }
            else
            {
                info((arg[i]), l);
            }
        }
    }
    else
    {
        if (r)
        {
            list_rec(".", h, l);
        }
        else
        {
            list_dir(".", h, l);
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
        else if (strcmp(trozos[0], "create") == 0)
        {
            create(trozos);
        }
        else if (strcmp(trozos[0], "delete") == 0)
        {
            del(trozos);
        }
        else if (strcmp(trozos[0], "list") == 0)
        {
            list(trozos);
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