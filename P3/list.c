#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"

#define MAXTAM 4096

// implementación de la lista y operaciones básicas

void crearLista(ListM *lista)
{
    lista->size = 0;
}

bool esListaVacia(ListM *lista)
{
    if (lista->size == 0)
        return true;
    else
        return false;
}

void insertar(ListM *lista, Data *data)
{
    lista->datos[lista->size] = data;
    lista->size++;
}

Data out(ListM *lista, int position)
{
    return *lista->datos[position];
}

int findTam(ListM *lista, int t)
{
    if (esListaVacia(lista))
        return -1;
    else
    {
        for (int i = 0; i <= lista->size; i++)
            if (lista->datos[i]->tam == t)
                return i;
        return -1;
    }
}

int findKey(ListM *lista, int k)
{
    if (esListaVacia(lista))
        return -1;
    else
    {
        for (int i = 0; i <= lista->size; i++)
            if (lista->datos[i]->arguments == k)
                return i;
        return -1;
    }
}

int findDirStr(ListM *lista, char dS[20])
{
    if (esListaVacia(lista))
        return -1;
    else
    {
        for (int i = 0; i <= lista->size; i++)
            if (strcmp(lista->datos[i]->directionStr, dS) == 0)
                return i;
        return -1;
    }
}

int findL(ListM *lista, char str[20])
{
    if (esListaVacia(lista))
        return -1;
    else
    {
        for (int i = 0; i <= lista->size; i++)
            if (strcmp(lista->datos[i]->name, str) == 0)
                return i;
        return -1;
    }
}

void deleteL(ListM *lista, int position)
{
    for (int i = position; i < lista->size; i++)
        lista->datos[i] = lista->datos[i + 1];
    lista->size--;
}