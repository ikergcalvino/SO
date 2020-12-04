typedef struct
{
    int tam;
    char name[20];
    char type[20];
    char arguments;
    void *direction;
    time_t timestamp;
    char directionStr[20];
} Data;

typedef struct
{
    Data *datos[4096];
    int size;
} ListM;

void crearLista(ListM *lista);
bool esListaVacia(ListM *lista);
void insertar(ListM *lista, Data *data);
Data out(ListM *lista, int position);
int findTam(ListM *lista, int t);
int findKey(ListM *lista, int k);
int findDirStr(ListM *lista, char dS[20]);
int findL(ListM *lista, char str[20]);
void deleteL(ListM *lista, int position);