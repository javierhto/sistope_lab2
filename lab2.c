#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h> //Unncoment in Linux
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <pthread.h>

// Definiciones de constantes
#define TRUE 1
#define FALSE 0
#define LECTURA 0
#define ESCRITURA 1

typedef struct Monitor
{
  char * buffer;
  int cantidad; //Cantidad de datos almacenados
  pthread_cond_t nolleno, novacio;
}Monitor;

typedef struct Visibilidad
{
  int u;
  int v;
  int real;
  int imag;
  int ruido;
  struct Visibilidad * siguiente;
}Visibilidad;

//Definición de variables globales
pthread_mutex_t mutex;
int discCant;
int discWidth;
int bFlag;
int tamanoBuffer;



//Función que inicializa un monitor
//Entrada: Hebra
//Salida: Monitor
Monitor * inicializarMonitor()
{
  Monitor * monitor;
  monitor->cantidad = 0;
  monitor->buffer = malloc(sizeof(char)*tamanoBuffer);
  pthread_cond_init(&monitor->nolleno, NULL);
  pthread_cond_init(&monitor->novacio, NULL);
  return monitor;
}

//Función que inciializa la ejecuión de una hebra
//Entrada:
//Salida: Nada, vacío
void * obtenerVisibilidades()
{
  printf("Hebra esperando hacer algo...\r\n");
  pthread_mutex_lock(&mutex);
  printf("Hebra haciendo algo\r\n");
}

int main(int argc, char* argv[])
{
    printf("\n\n##### Inicio de la ejecucion PADRE #####\n\n");
    //Manejo de las banderas
    //Variables de entrada
    bFlag = FALSE;          //-b Bandera indicadora de imprimir datos en pantalla
    char * fileIn = NULL;       //-i Nombre del archivo de entrada
    char * fileOut = NULL;      //-o Nombre del archivo de salida
    discCant = 0;           //-n Cantidad de discos
    discWidth = 0;          //-d Ancho de cada disco
    tamanoBuffer = 0;       //-s tamaño del vuffer de cada monitor
    //Variables de procesamiento.
    FILE* fs;

    if (argc < 9) //Si hay menos de 9 argumentos, termina la ejecución del programa y entrega mensaje de error
    {
        printf("La cantidad de argumentos es menor a la requerida\n");
        return 0;
    }
    else
    {
        int c; //Auxiliar
        while ((c = getopt(argc,argv,"i:o:n:d:s:b")) != -1)
        {
            switch(c)
            {
                case 'i':
                    fileIn = optarg;
                    break;

                case 'o':
                    fileOut = optarg;
                    break;

                case 'n':
                    sscanf(optarg, "%i", &discCant);
                    break;

                case 'd':
                    sscanf(optarg, "%i", &discWidth);
                    break;

                case 's':
                    sscanf(optarg, "%i", &tamanoBuffer);
                    break;

                case 'b':
                    bFlag = TRUE;
                    break;
            }
        }
    }
    //CONDICION QUE EVITA VALORES NEGATIVOS EN LA ENTRADA DE LOS ARCHIVOS.
    if(discCant < 1 || discWidth < 1){
      printf("La cantidad de discos y/o ancho de estos no puede ser menor a 1.\r\nIntente nuevamente.\r\n");
      exit(0);
    }
    if(strcmp(fileOut, fileIn) == 0){
      printf("El nombre del archivo de entrada no puede ser igual al archivo de salida.\r\nIntente nuevamente.\r\n");
      exit(0);
    }
    if(tamanoBuffer < 1){
      printf("El tamano del buffer no puede ser menor a 1.\r\nIntente nuevamente.\r\n");
      exit(0);
    }

    //DEBUG
    printf("Iniciando procesamiento con %i discos...\r\n", discCant);


    //Se crea un arreglo de hebras del tamaño de la cantidad de discos
    pthread_t threads[discCant];
    //Se inicializa un mutex
    pthread_mutex_init(&mutex, NULL);

    int i;

    for(i=0; i<discCant; i++) //Se crean tantas hebras como discos
    {
      pthread_create(&threads[i], NULL, obtenerVisibilidades, NULL); //Utilización: Pthread_create: (direccion de memoria de la hebra a crear, NULL, función vacia que iniciará la hebra, parámetros de la función)
      //printf("Mutex %i: %i", i);
    }


    printf("\n\n##### Fin de la ejecucion PADRE #####\n\n");
    return 0;
}
