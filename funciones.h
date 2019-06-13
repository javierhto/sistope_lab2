#ifndef funciones_h
#define funciones_h
// Definiciones de constantes
#define TRUE 1
#define FALSE 0
#define LECTURA 0
#define ESCRITURA 1
#define ABIERTO 1
#define CERRADO 0

typedef struct Visibilidad
{
  int totalVisibilidades;
  double mediaReal;
  double mediaImaginaria;
  double ruidoTotal;
  double potencia;
}Visibilidad;

typedef struct Buffer
{
    int id;
    int cantidad;
    int estado;
    char ** data;
    int full;
    int empty;
    pthread_mutex_t mutex;
    pthread_cond_t notFull, notEmpty;
}Buffer;

char* readLine(FILE * file);

void writeFile(Visibilidad* datosVisibilidad, char* nombreArchivo, int numDisco);

int checkDestination(double coordV, double coordU, int discWidth, int discCant);

void inicializarCharArray(char* array, int largo);

int obtenerVisibilidadRecibida(char* visibilidad, int discWidth, int discCant);

void EnterSC(pthread_mutex_t * mutex);

void ExitSC(pthread_mutex_t * mutex);

double* obtenerDatosVisibilidad(char* visibilidad);

Buffer * inicializarBuffer();

Visibilidad * inicializarVisibilidad();

double* procesarDatosBuffer(double real, double imaginaria, double ruido, int totalVisibilidades, Buffer *b);

void almacenarDatos(double mediaReal, double mediaImaginaria, double ruido, double potencia, int totalVisibilidades, Visibilidad *visibilidad);

void anadirDato(Buffer * b, char * line);

void vaciarBuffer(Buffer * b);

void vaciarBufferSinReasignar(Buffer * b);

void mostrarVisibilidades();


#endif
