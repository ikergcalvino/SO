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
#include <signal.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "list.h"
#include "listproc.h"

#define T_REC 2048
#define MAXTAM 4096
#define LEERCOMPLETO ((ssize_t)-1)

// variables globales para el show
int global_v1, global_v2, global_v3;

void showListaMemoria(ListM *lista, char *mode);

/*
 * Íker García Calviño <iker.gcalvino>
 *  DNI: 34292367B
 * Juan Toirán Freire <juan.tfreire>
 *  DNI: 34291468D
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

// funciones aux p2
/*********************************************************/

void *ObtenerMemoriaShmget(key_t clave, size_t tam, Data *data)
{
    void *p;
    int aux, id, flags = 0777;
    struct shmid_ds s;
    if (tam)                                  /*si tam no es 0 la crea en modo exclusivo */
        flags = flags | IPC_CREAT | IPC_EXCL; /*si tam es 0 intenta acceder a una ya creada*/
    if (clave == IPC_PRIVATE)                 /*no nos vale*/
    {
        errno = EINVAL;
        return NULL;
    }
    if ((id = shmget(clave, tam, flags)) == -1)
        return (NULL);
    if ((p = shmat(id, NULL, 0)) == (void *)-1)
    {
        aux = errno; /*si se ha creado y no se puede mapear*/
        if (tam)     /*se borra */
            shmctl(id, IPC_RMID, NULL);
        errno = aux;
        return (NULL);
    }
    shmctl(id, IPC_STAT, &s); /* Guardar En Direcciones de Memoria Shared (p, s.shm_segsz, clave.....);*/
    data->direction = p;
    data->tam = s.shm_segsz;
    sprintf(data->name, "%p", data->direction);
    sprintf(data->directionStr, "%p", data->direction);
    strcpy(data->type, "shared");
    data->timestamp = time(0);
    data->arguments = clave;
    return (p);
}

void Cmd_AlocateCreateShared(char *arg[], ListM *lista) /*arg[0] is the keyand arg[1] is the size*/
{
    void *p;
    key_t k;
    Data data, copy;
    size_t tam = 0;
    if (arg[3] == NULL || arg[4] == NULL)
    { /*Listar Direcciones de Memoria Shared;*/
        showListaMemoria(lista, "-shared");
        return;
    }
    k = (key_t)atoi(arg[3]);
    if (arg[4] != NULL)
        tam = (size_t)atoll(arg[4]);
    if ((p = ObtenerMemoriaShmget(k, tam, &data)) == NULL)
        perror("Imposible obtener memoria shmget");
    else
    {
        printf("Allocated shared memory (key %d) at %p\n", k, p);
        memcpy(&copy, &data, sizeof(Data));
        insertar(lista, &copy);
    }
}

/************************************************************************/ /************************************************************************/

void *MmapFichero(char *fichero, int protection, Data *data)
{
    int df, map = MAP_PRIVATE, modo = O_RDONLY;
    struct stat s;
    void *p;
    if (protection & PROT_WRITE)
        modo = O_RDWR;
    if (stat(fichero, &s) == -1 || (df = open(fichero, modo)) == -1)
        return NULL;
    if ((p = mmap(NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED)
        return NULL;
    /*Guardar Direccion de Mmap (p, s.st_size,fichero,df......);*/
    data->direction = p;
    data->tam = s.st_size;
    strcpy(data->name, fichero);
    strcpy(data->type, "mmap");
    sprintf(data->directionStr, "%p", data->direction);
    data->timestamp = time(0);
    data->arguments = df;
    return p;
}

void Cmd_AllocateMmap(char *arg[], ListM *lista) /*arg[0] is the file name and arg[1] is the permissions*/
{
    int protection = 0;
    char *perm;
    Data data, copy;
    void *p;

    if (arg[3] == NULL)
    { /*Listar Direcciones de Memoria mmap;*/
        showListaMemoria(lista, "-mmap");
        return;
    }
    if ((perm = arg[4]) != NULL && strlen(perm) < 4)
    {
        if (strchr(perm, 'r') != NULL)
            protection |= PROT_READ;
        if (strchr(perm, 'w') != NULL)
            protection |= PROT_WRITE;
        if (strchr(perm, 'x') != NULL)
            protection |= PROT_EXEC;
    }
    if ((p = MmapFichero(arg[3], protection, &data)) == NULL)
        perror("Imposible mapear fichero");
    else
    {
        printf("file %s mapped at %p\n", arg[3], p);
        memcpy(&copy, &data, sizeof(Data));
        insertar(lista, &copy);
    }
}

ssize_t LeerFichero(char *fich, void *p, ssize_t n)
{ /*n=-1 indica que se lea todo*/
    ssize_t nleidos, tam = n;
    int df, aux;
    struct stat s;
    if (stat(fich, &s) == -1 || (df = open(fich, O_RDONLY)) == -1)
        return ((ssize_t)-1);
    if (n == LEERCOMPLETO)
        tam = (ssize_t)s.st_size;
    if ((nleidos = read(df, p, tam)) == -1)
    {
        aux = errno;
        close(df);
        errno = aux;
        return ((ssize_t)-1);
    }
    close(df);
    return (nleidos);
}

/*********************************************************************/

void Cmd_deletekey(char *args[]) /*arg[0] points to a str containing the key*/
{
    key_t clave;
    int id;
    char *key = args[3];
    if (key == NULL || (clave = (key_t)strtoul(key, NULL, 10)) == IPC_PRIVATE)
    {
        printf("   rmkey  clave_valida\n");
        return;
    }
    if ((id = shmget(clave, 0, 0666)) == -1)
    {
        perror("shmget: imposible obtener memoria compartida");
        return;
    }
    if (shmctl(id, IPC_RMID, NULL) == -1)
        perror("shmctl: imposible eliminar memoria compartida\n");
}

void Cmd_dopmap(char *args[]) /*no arguments necessary*/
{
    pid_t pid;
    char elpid[32];
    char *argv[3] = {"pmap", elpid, NULL};
    sprintf(elpid, "%d", (int)getpid());
    if ((pid = fork()) == -1)
    {
        perror("Imposible crear proceso");
        return;
    }
    if (pid == 0)
    {
        if (execvp(argv[0], argv) == -1)
            perror("cannot execute pmap");
        exit(1);
    }
    waitpid(pid, NULL, 0);
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
    free(strdup(element));
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
        for (i = 0; i <= N; i++)
            printf("%i: %s", i, (list->elements[i])->comando);
    else if (N == list->size)
        for (i = 0; i < N; i++)
            printf("%i: %s", i, (list->elements[i])->comando);
    else
        printf("Error: position %i not valid.\n", N);
}

// funciones auxiliares e implementación del shell

void process(char entrada[], List *lista, ListM *listamemoria, bool *bucle);

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
        printf("Error: argument not valid.\n");
}

void pid(char *arg)
{
    if (arg == NULL)
        printf("PID of process: %i\n", getpid());
    else
        printf("Error: argument not valid.\n");
}

void ppid(char *arg)
{
    if (arg == NULL)
        printf("PID of parent process: %i\n", getppid());
    else
        printf("Error: argument not valid.\n");
}

void pwd(char *arg)
{
    char cwd[MAXTAM];
    if (arg == NULL)
        printf("%s\n", getcwd(cwd, MAXTAM));
    else
        printf("Error: argument not valid.\n");
}

void cdir(char *arg)
{
    if (arg == NULL)
        pwd(NULL);
    else if (chdir(arg) != 0)
        printf("Error: %s.\n", strerror(errno));
}

void date(char *arg)
{
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    if (arg == NULL)
        printf("%1.2d/%1.2d/%1.2d\n", local->tm_mday, local->tm_mon, 1900 + local->tm_year);
    else
        printf("Error: argument not valid.\n");
}

void tiempo(char *arg)
{
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    if (arg == NULL)
        printf("%d:%d:%d\n", local->tm_hour, local->tm_min, local->tm_sec);
    else
        printf("Error: argument not valid.\n");
}

void historic(List *lista, ListM *lmemoria, char *arg)
{
    if (arg == NULL)
        PrintList(lista, (lista->size));
    else if (strcmp(arg, "-c") == 0)
        ClearList(lista);
    else if (isdigit(arg[1]))
        PrintList(lista, atoi(++arg));
    else if (arg[1] == 'r' && isdigit(arg[2]))
    {
        arg++;
        if (atoi(++arg) < lista->size)
        {
            printf("%s", lista->elements[atoi(arg)]->comando);
            process(strdup(lista->elements[atoi(arg)]->comando), lista, lmemoria, false);
            free(strdup(lista->elements[atoi(arg)]->comando));
        }
        else
            printf("Error: command at position %i not valid.\n", atoi(arg));
    }
    else
        printf("Error: argument not valid.\n");
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
                printf("\t%s\n", file);
        }
        else
            printf("%ld\t%s \n", st.st_size, file);
    }
    else
        printf("%s\n", strerror(errno));
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
                info(act->d_name, lon);
            else if ((strcmp(act->d_name, ".") != 0) && (strcmp(act->d_name, "..") != 0))
                info(act->d_name, lon);
        }
        closedir(directory);
    }
    else
        printf("%s\n", strerror(errno));
}

void create(char *args[])
{
    if (args[1] == NULL)
        list_dir(".", false, false);
    else if (strcmp(args[1], "-dir") == 0)
    {
        if (mkdir(args[2], S_IRWXU | S_IRWXG) == -1)
            printf("%s\n", strerror(errno));
    }
    else if (open(args[1], O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
        printf("%s\n", strerror(errno));
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
            printf("Error: %s.\n", strerror(errno));
        while ((act = readdir(directory)) != NULL)
        {
            if ((strcmp(act->d_name, ".") != 0) && (strcmp(act->d_name, "..") != 0))
            {
                if (stat(act->d_name, &st) == -1)
                    printf("%s\n", strerror(errno));
                else if (LetraTF(st.st_mode) == 'd')
                {
                    if (unlink(act->d_name) == -1)
                        printf("%s\n", strerror(errno));
                }
                else if (LetraTF(st.st_mode) == '-')
                {
                    if (remove(act->d_name) == -1)
                        printf("%s\n", strerror(errno));
                }
                del_rec(act->d_name);
            }
        }
        chdir("..");
    }
    else
    {
        printf("%s\n", strerror(errno));
    }
    closedir(directory);
    if (rmdir(direct) == -1)
        printf("%s\n", strerror(errno));
}

void del(char *args[])
{
    int i;
    char directorio[MAXTAM];

    getcwd(directorio, MAXTAM);

    if (args[1] == NULL)
        list_dir(".", false, false);
    else if (strcmp(args[1], "-rec") == 0)
        for (i = 1; args[i] != NULL; i++)
        {
            del_rec(args[i]);
            if (chdir(directorio) != 0)
                printf("Error: %s.\n", strerror(errno));
        }
    else
        for (i = 1; args[i] != NULL; i++)
            if (remove(args[i]) == -1)
                printf("%s\n", strerror(errno));
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
                if ((strcmp(act->d_name, ".") != 0) && (strcmp(act->d_name, "..") != 0))
                {
                    if ((LetraTF(st.st_mode) == 'd'))
                    {
                        printf("\n");
                        if (chdir(act->d_name) != 0)
                            printf("Error: %s.\n", strerror(errno));
                        else
                        {
                            list_rec(act->d_name, hid, lon);
                            chdir("..");
                        }
                    }
                }
            }
            closedir(directory);
        }
        else
            printf("Error: %s.\n", strerror(errno));
    }
}

void list(char *args[])
{
    int i;
    int flags = 1;
    bool l = false, d = false, h = false, r = false;

    for (i = 1; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "-long") == 0)
        {
            l = true;
            flags++;
        }
        else if (strcmp(args[i], "-dir") == 0)
        {
            d = true;
            flags++;
        }
        else if (strcmp(args[i], "-hid") == 0)
        {
            h = true;
            flags++;
        }
        else if (strcmp(args[i], "-rec") == 0)
        {
            r = true;
            flags++;
        }
        else
            break;
    }

    if (args[flags] != NULL)
    {
        for (i = flags; args[i] != NULL; i++)
        {
            if (d)
            {
                if (r)
                {
                    if (chdir(args[i]) != 0)
                        list_rec(".", h, l);
                    else
                    {
                        list_rec(".", h, l);
                        chdir("..");
                    }
                }
                else
                {
                    if (chdir(args[i]) != 0)
                        printf("Error: %s.\n", strerror(errno));
                    else
                    {
                        list_dir(".", h, l);
                        chdir("..");
                    }
                }
            }
            else
                info((args[i]), l);
        }
    }
    else
    {
        if (r)
            list_rec(".", h, l);
        else
            list_dir(".", h, l);
    }
}

void showListaMemoria(ListM *lista, char *mode)
{
    struct tm *information;
    char buffer[25];
    Data datos;
    int p = 0;

    if ((strcmp(mode, "-mmap") == 0) && (lista->size > 0))
    {
        do
        {
            datos = out(lista, p);
            information = localtime(&datos.timestamp);
            strftime(buffer, 50, "%a %b %d %H:%M:%S %Y", information);
            if (strcmp(datos.type, "mmap") == 0)
                printf("%s %s %d %s (descriptor %d)\n", datos.directionStr, buffer, datos.tam, datos.name, datos.arguments);
            p++;
        } while (p != lista->size);
    }
    else if ((strcmp(mode, "-malloc") == 0) && (lista->size > 0))
    {
        do
        {
            datos = out(lista, p);
            information = localtime(&datos.timestamp);
            strftime(buffer, 50, "%a %b %d %H:%M:%S %Y", information);
            if (strcmp(datos.type, "malloc") == 0)
                printf("%s %s %d %s\n", datos.name, buffer, datos.tam, datos.type);
            p++;
        } while (p < lista->size);
    }
    else if ((strcmp(mode, "-shared") == 0) && (lista->size > 0))
    {
        do
        {
            datos = out(lista, p);
            information = localtime(&datos.timestamp);
            strftime(buffer, 50, "%a %b %d %H:%M:%S %Y", information);
            if (strcmp(datos.type, "shared") == 0)
                printf("%s %s %d %s %s (key %d)\n", datos.name, buffer, datos.tam, datos.name, datos.type, datos.arguments);
        } while (p != lista->size);
    }
    else if ((strcmp(mode, "-all") == 0) && (lista->size > 0))
    {
        do
        {
            datos = out(lista, p);
            information = localtime(&datos.timestamp);
            strftime(buffer, 50, "%a %b %d %H:%M:%S %Y", information);
            if (strcmp(datos.type, "mmap") == 0)
                printf("%s %s %d %s (descriptor %d)\n", datos.directionStr, buffer, datos.tam, datos.name, datos.arguments);
            else if (strcmp(datos.type, "malloc") == 0)
                printf("%s %s %d %s\n", datos.name, buffer, datos.tam, datos.type);
            else if (strcmp(datos.type, "shared") == 0)
                printf("%s %s %d %s %s (key %d)\n", datos.name, buffer, datos.tam, datos.name, datos.type, datos.arguments);
        } while (p != lista->size);
    }
}

void AllocatedShared(char *args[], ListM *lista)
{
    void *p;
    Data data, copy;
    key_t key;
    size_t tam = 0;

    if (args[3] == NULL)
        showListaMemoria(lista, "-shared");
    else
    {
        key = (key_t)atoi(args[3]);
        if ((p = ObtenerMemoriaShmget(key, tam, &data)) == NULL)
            perror("Not possible to obtain shmget memory.");
        else
        {
            printf("Shmget memory in key %d assigned in %p\n", key, p);
            memcpy(&copy, &data, sizeof(Data));
            insertar(lista, &copy);
        }
    }
}

void allocate(char *args[], ListM *lista)
{
    Data data, copy;

    if (args[2] == NULL)
    {
        showListaMemoria(lista, "-all");
    }
    else if (strcmp(args[2], "-malloc") == 0)
    {
        if (args[3] == NULL)
        {
            showListaMemoria(lista, "-malloc");
        }
        data.tam = atoi(args[3]);
        data.direction = malloc(data.tam);
        if (data.direction == NULL)
            printf("Memory not possible with malloc: %s", strerror(errno));
        else
        {
            sprintf(data.name, "%p", data.direction);
            sprintf(data.directionStr, "%p", data.direction);
            data.timestamp = time(0);
            strcpy(data.type, "malloc");
            printf("allocated %d bytes at %s\n", data.tam, data.name);
            memcpy(&copy, &data, sizeof(Data));
            insertar(lista, &copy);
        }
    }
    else if (strcmp(args[2], "-mmap") == 0)
        Cmd_AllocateMmap(args, lista);
    else if (strcmp(args[2], "-createshared") == 0)
        Cmd_AlocateCreateShared(args, lista);
    else if (strcmp(args[2], "-shared") == 0)
        AllocatedShared(args, lista);
    else
    {
        printf("tiritiritiri\n");
    }
}

void deallocate(char *args[], ListM *lista)
{
    int p;
    Data data;

    if (args[2] == NULL)
    {
        showListaMemoria(lista, "-all");
    }
    else if (strcmp(args[2], "-malloc") == 0)
    {
        if (args[3] == NULL)
        {
            showListaMemoria(lista, "-malloc");
        }
        else
        {
            p = findTam(lista, atoi(args[3]));
            if (p == 0)
            {
                printf("block of that size not malloced\n");
            }
            else
            {
                data = out(lista, p);
                free(data.direction);
                deleteL(lista, p);
            }
        }
    }
    else if (strcmp(args[2], "-mmap") == 0)
        if (args[3] == NULL)
        {
            showListaMemoria(lista, "-mmap");
        }
        else
        {
            p = findTam(lista, atoi(args[3]));
            if (p == 0)
            {
                printf("file not maped\n");
            }
            else
            {
                data = out(lista, p);
                munmap(data.direction, data.tam);
                deleteL(lista, p);
            }
        }
    else if (strcmp(args[2], "-shared") == 0)
        if (args[3] == NULL)
        {
            showListaMemoria(lista, "-shared");
        }
        else
        {
            p = findTam(lista, atoi(args[3]));
            if (p == 0)
            {
                printf("block not maped in process\n");
            }
            else
            {
                data = out(lista, p);
                shmdt(data.direction);
                deleteL(lista, p);
            }
        }
    else
    {
        p = findDirStr(lista, args[2]);
        if (p == 0)
        {
            printf("direction %s not assigned neither malloc, shared or mmap\n", args[2]);
        }
        else
        {
            data = out(lista, p);
            if (!strcmp(data.type, "malloc"))
                free(data.direction);
            if (!strcmp(data.type, "mmap"))
                munmap(data.direction, data.tam);
            if (!strcmp(data.type, "shared"))
                shmdt(data.direction);
            deleteL(lista, p);
        }
    }
}

void show(char *args[], ListM *lista)
{
    int v1, v2, v3;
    if (strcmp(args[1], "-show") == 0)
    {
        if (args[2] == NULL)
        {
            printf("\t- program functions:\t [%p] [%p] [%p]\n", &authors, &pid, &ppid);
            printf("\t- global variables:\t [%p] [%p] [%p]\n", &global_v1, &global_v2, &global_v3);
            printf("\t- local variables:\t [%p] [%p] [%p]\n", &v1, &v2, &v3);
        }
        else
            showListaMemoria(lista, args[2]);
    }
    else if (strcmp(args[1], "-show-vars") == 0)
    {
        printf("\t- global variables:\t [%p] [%p] [%p]\n", &global_v1, &global_v2, &global_v3);
        printf("\t- local variables:\t [%p] [%p] [%p]\n", &v1, &v2, &v3);
    }
    else if (strcmp(args[1], "-show-funcs") == 0)
    {
        printf("\t- program functions:\t [%p] [%p] [%p]\n", &authors, &pid, &ppid);
        printf("\t- library functions:\t [%p] [%p] [%p]\n", &printf, &strcmp, &free);
    }
    else if (strcmp(args[1], "-dopmap") == 0)
    {
        // -dopmap
    }
}

void memdump(char *args[])
{
    int i;
    void *p;

    if (args[1] != NULL)
    {
        p = (void *)strtoull(args[1], NULL, 16);
        if (args[2] == NULL)
        {
            for (i = 0; i < 25; i++)
                printf("%c  ", *(char *)(p + i));
            printf("\n");
            for (i = 0; i < 25; i++)
                printf("%2x ", *(char *)(p + i));
            printf("\n");
        }
        else
        {
            for (i = 0; i < atoi(args[2]); i++)
                printf("%c  ", *(char *)(p + i));
            printf("\n");
            for (i = 0; i < atoi(args[2]); i++)
                printf("%2x ", *(char *)(p + i));
            printf("\n");
        }
    }
}

void memfill(char *args[])
{
    int i;
    void *p;

    if (args[1] != NULL)
    {
        p = (void *)strtoull(args[1], NULL, 16);
        if (args[2] == NULL)
        {
            for (i = 0; i < 128; i++)
                (*(char *)(p + i)) = 65;
        }
        if (args[3] == NULL)
        {
            for (i = 0; i < atoi(args[2]); i++)
                (*(char *)(p + i)) = 65;
        }
        for (i = 0; i < atoi(args[2]); i++)
            (*(char *)(p + i)) = args[3][0];
    }
}

void doRecursiva(int n)
{
    char automatico[T_REC];
    static char estatico[T_REC];

    printf("parametro n:%d en %p\n", n, &n);
    printf("array estatico en:%p \n", estatico);
    printf("array automatico en %p\n", automatico);
    n--;
    if (n > 0)
        doRecursiva(n);
}

void readfile(char *args[])
{
    void *p;
    ssize_t fleidos;

    if (args[2] == NULL)
        printf("missing arguments.\n");
    else if (args[3] == NULL)
    {
        p = (void *)strtoull(args[2], NULL, 16);
        fleidos = LeerFichero(args[1], p, LEERCOMPLETO);
        printf("readed %ld bytes in %s since %s \n", fleidos, args[1], args[2]);
    }
    else
    {
        p = (void *)strtoull(args[2], NULL, 16);
        fleidos = LeerFichero(args[1], p, atoi(args[3]));
        printf("readed %ld bytes in %s since %s \n", fleidos, args[1], args[2]);
    }
    return;
}

ssize_t EscribirFichero(char *fich, void *p, ssize_t n, int f)
{ /*n=-1 indica que se lea todo*/
    ssize_t nleidos, tam = n;
    int df, aux;
    struct stat s;
    if (stat(fich, &s) == -1 || (df = open(fich, f, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    {
        printf("Not possible write in file: File exists\n");
        return LEERCOMPLETO;
    }
    else
    {
        if ((df = open(fich, f)) == -1)
        {
            perror("Not possible write in file:");
            return LEERCOMPLETO;
        }
    }
    if ((nleidos = write(df, p, tam)) == -1)
    {
        aux = errno;
        close(df);
        errno = aux;
        return LEERCOMPLETO;
    }
    close(df);
    return (nleidos);
}

void writefile(char *args[])
{
    void *p;
    ssize_t fescritos;

    if (args[2] == NULL)
    {
        printf("missing arguments.\n");
        return;
    }
    p = (void *)strtoull(args[2], NULL, 16);
    if ((args[4] != NULL) && (!strcmp(args[4], "-o")))
    {
        fescritos = EscribirFichero(args[1], p, atoi(args[3]), O_WRONLY | O_CREAT | O_TRUNC);
        printf("written %ld bytes in %s since %s \n", fescritos, args[1], args[2]);
    }
    else
    {
        if ((fescritos = EscribirFichero(args[1], p, atoi(args[3]), O_WRONLY | O_CREAT | O_TRUNC)) != -1)
            printf("written %ld bytes in %s since %s \n", fescritos, args[1], args[2]);
    }
    return;
}

void getterpriority(char *pid) {}
void setterpriority(char *pid, char *value) {}

void getteruid() {}
void setteruid(char *flag, char *id) {}

void ffork() {}

void eexec(char *args[]) {}

void pplano(char *args[]) {}

void splano(char *args[]) {}

void listprocs() {}

void proc(char *flag, char *id) {}

void deleteprocs(char *flag) {}

int trocear(char *cadena, char *trozos[])
{
    int i = 1;
    if ((trozos[0] = strtok(cadena, " \n\t")) == NULL)
        return 0;
    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL)
        i++;
    return i;
}

void process(char entrada[], List *lista, ListM *listamemoria, bool *bucle)
{
    char *trozos[15];
    int acceso = trocear(entrada, trozos);
    if (acceso > 0)
    {
        if (strcmp(trozos[0], "authors") == 0)
            authors(trozos[1]);
        else if (strcmp(trozos[0], "getpid") == 0)
            pid(trozos[1]);
        else if (strcmp(trozos[0], "getppid") == 0)
            ppid(trozos[1]);
        else if (strcmp(trozos[0], "pwd") == 0)
            pwd(trozos[1]);
        else if (strcmp(trozos[0], "chdir") == 0)
            cdir(trozos[1]);
        else if (strcmp(trozos[0], "date") == 0)
            date(trozos[1]);
        else if (strcmp(trozos[0], "time") == 0)
            tiempo(trozos[1]);
        else if (strcmp(trozos[0], "historic") == 0)
            historic(lista, listamemoria, trozos[1]);
        else if (strcmp(trozos[0], "create") == 0)
            create(trozos);
        else if (strcmp(trozos[0], "delete") == 0)
            del(trozos);
        else if (strcmp(trozos[0], "list") == 0)
            list(trozos);
        else if (strcmp(trozos[0], "memory") == 0)
        {
            if (strcmp(trozos[1], "-allocate") == 0)
                allocate(trozos, listamemoria);
            else if (strcmp(trozos[1], "-deallocate") == 0)
                deallocate(trozos, listamemoria);
            else
                show(trozos, listamemoria);
        }
        else if (strcmp(trozos[0], "memdump") == 0)
            memdump(trozos);
        else if (strcmp(trozos[0], "memfill") == 0)
            memfill(trozos);
        else if (strcmp(trozos[0], "recurse") == 0)
            doRecursiva(atoi(trozos[1]));
        else if (strcmp(trozos[0], "readfile") == 0)
            readfile(trozos);
        else if (strcmp(trozos[0], "writefile") == 0)
            writefile(trozos);
        else if (strcmp(trozos[0], "getpriority") == 0)
            getterpriority(trozos[1]);
        else if (strcmp(trozos[0], "setpriority") == 0)
            setterpriority(trozos[1], trozos[2]);
        else if (strcmp(trozos[0], "getuid") == 0)
            getteruid();
        else if (strcmp(trozos[0], "setuid") == 0)
            setteruid(trozos[1], trozos[2]);
        else if (strcmp(trozos[0], "fork") == 0)
            ffork();
        else if (strcmp(trozos[0], "exec") == 0)
            eexec(trozos);
        else if (strcmp(trozos[0], "foreground") == 0)
            pplano(trozos);
        else if (strcmp(trozos[0], "background") == 0)
            splano(trozos);
        else if (strcmp(trozos[0], "listprocs") == 0)
            listprocs();
        else if (strcmp(trozos[0], "proc") == 0)
            proc(trozos[1], trozos[2]);
        else if (strcmp(trozos[0], "deleteprocs") == 0)
            deleteprocs(trozos[1]);
        else if (strcmp(trozos[0], "quit") == 0 || strcmp(trozos[0], "end") == 0 || strcmp(trozos[0], "exit") == 0)
            *bucle = true;
        else
            printf("Error: command not found.\n");
    }
}

void reader(List *lista, char command[])
{
    fgets(command, MAXTAM, stdin);
    if ((int)command[0] != 10)
        InsertInList(lista, command);
}

int main(int argc, char const *argv[])
{
    List *lista = malloc(sizeof(List));
    ListM *memoria = malloc(sizeof(ListM));
    char entrada[MAXTAM];
    CreateList(lista);
    bool bucle = false;
    while (!bucle)
    {
        prompt();
        reader(lista, entrada);
        process(entrada, lista, &memoria, &bucle);
    }
    ClearList(lista);
    free(memoria);
    free(lista);
    return 0;
}