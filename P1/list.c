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

int main(int argc, char *args[])
{
    list(args);
    return 0;
}