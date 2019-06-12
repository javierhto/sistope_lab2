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
#include "archivo.h"
#include "funciones.h"

// Definiciones de constantes
#define TRUE 1
#define FALSE 0
#define LECTURA 0
#define ESCRITURA 1
#define ABIERTO 1
#define CERRADO 0

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

typedef struct Buffer
{
    int cantidad;
    int estado;
    char ** data;
}Buffer;

//Definición de variables globales
//pthread_mutex_t mutex;
int discCant;
int discWidth;
int bFlag;
int tamanoBuffer;


//Función que inicializa el buffer de un monitor
//Entrada: Nada - utiliza datos globales
//Salida: Puntero a buffer del tamaño instanciado
Buffer * inicializarBuffer()
{
    Buffer * buffer = (Buffer*)malloc(sizeof(Buffer));
    buffer->cantidad = 0;
    buffer->estado = ABIERTO;
    buffer->data = (char**)malloc(sizeof(char*)*tamanoBuffer);
    return buffer;
}

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
void * hebra(void * buffer)
{
    //Datos necesaarios para el cálculo
    double mediaReal = 0;
    double mediaImaginaria = 0;
    double ruido = 0;
    int totalVisibilidades = 0;
    int cantidadVisibilidades = 0;

    //Casteo del buffer entregado
    Buffer * bufferLocal = (Buffer*)buffer;

    while(bufferLocal->estado == ABIERTO)
    {
        if(bufferLocal->cantidad == tamanoBuffer)
        {
            printf("Se leeran los datos del buffer...\r\n");
            bufferLocal->cantidad = 0;
        }
    }
    printf("Fin \r\n");
}

//Función que añade un dato a un buffer
//Entrada: Puntero al buffer que se añadirá un dato
//Salida: Vacío
void anadirDato(Buffer * b, char * line)
{
    b->data[b->cantidad] = line;
    b->cantidad = b->cantidad + 1;  
}

int main(int argc, char* argv[])
{
    printf("\n\n##### Inicio de la ejecucion PADRE #####\n\n");
    //Manejo de las banderas
    //Variables de entrada
    bFlag = FALSE;              //-b Bandera indicadora de imprimir datos en pantalla
    char * fileIn = NULL;       //-i Nombre del archivo de entrada
    char * fileOut = NULL;      //-o Nombre del archivo de salida
    discCant = 0;               //-n Cantidad de discos
    discWidth = 0;              //-d Ancho de cada disco
    tamanoBuffer = 0;           //-s tamaño del vuffer de cada monitor
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
    //Verificacion de buffer
    if(tamanoBuffer < 1){
      printf("El tamano del buffer no puede ser menor a 1.\r\nIntente nuevamente.\r\n");
      exit(0);
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

    //Se crea un arreglo de buffers en dónde se enviarán los datos a las hebras
    Buffer ** buffers = (Buffer**)malloc(sizeof(Buffer*)*discCant);

    //Se inicializa un mutex
    //pthread_mutex_init(&mutex, NULL);

    int i;
    for(i=0; i<discCant; i++) //Se crean tantas hebras como discos
    {
      buffers[i] = inicializarBuffer();  
      pthread_create(&threads[i], NULL, hebra, (void*)buffers[i]); //Utilización: Pthread_create: (direccion de memoria de la hebra a crear, NULL, función vacia que iniciará la hebra, parámetros de la función)
    }

    //En este momento se crearon DiscCant hebras y están esperando las lecturas

    //Datoshijos:
    //0. media real
    //1. imaginaria
    //2. portencia
    //3. ruido
    //4. vis totales
    fs = fopen(fileIn, "r");
    int j;
    if(fs == NULL){
       printf("File %s does not exist.\r\n", fileIn);
       exit(0);
    }
    int count = 1;
    printf("Procesando linea: \r\n");
    while(!feof(fs)){
       char * line = readLine(fs); //Leemos cada linea del archivo en cuestion.
       if(line[0] == '\0'){
         //AQUI ES CUANDO SE AVISA A LOS HIJOS DE FIN CERRANDO LOS BUFFERS
         //Y SE LES PIDE LOS DATOS CALCULADOS.
         for(j = 0; j < discCant; j++){
           buffers[i]->estado = CERRADO;
         }
        
         //PLAN: RECIBIR LOS DATOS DE LOS HIJOS Y LUEGO ALMACENARLO EN UN ARCHIVO.
       }
       else{
        //AQUI SE LES ENTREGA LINEA A LINEA LOS DATOS DE ENTRADA.
        //A CADA HIJO QUE TENGAMOS.
        //PLAN: ENVIAR LINE AL HIJO SELECCIONADO EN DISC MEDIANTE PIPE.
        int disc = obtenerVisibilidadRecibida(line, discWidth, discCant);
        printf("Enviando dato a disco: %i\r\n", disc);
        if(disc >= 0)
        {
          anadirDato(buffers[disc], line);
        }
        //Esto permite hacer conocer al usuario que linea del archivo el programa esta leyendo.
        //printf("\b\b\b\b\b\b\b\b\b");
        //fflush(stdout);
        //printf("%.7d", count);
        //fflush(stdout);
        //count = count + 1;
       }
       printf("%i.-Leyendo: %s\n\r", count, line);
       count = count + 1;
       free(line);
    }


    //FORZAMOS AL PADRE A ESPERAR POR TODOS SUS HIJOS
    //ESCRIBIMOS EN EL ARCHIVO LOS DATOS OBTENIDOS POR LOS HIJOS.
    for(i = 0; i < discCant; i++){
        pthread_join(threads[i], NULL);
    }


    //AQUI SE ENTREGA LOS RESULTADOS POR PANTALLA EN CASO DE QUE EL FLAG SEA VERDAD.
    //[Not yet]
    /*
    if(bFlag)
    {
        int total = 0;
        printf("\r\n");
        for(i = 0; i < discCant; i++)
        {
            printf("Soy el hijo, de pid %i y procese %i visibilidades\r\n", childs[i], (int)dataHijos[i][4]);
            total = total + (int)dataHijos[i][4];
        }
        printf("Total de visibilidades procesadas por mis hijos: %i\r\n", total);

    }

    //Antes de finalizar el programa, liberamos la memoria.
    //Liberamos la memoria del arreglo doble de Double.
    */

    printf("\n\n##### Fin de la ejecucion PADRE #####\n\n");
    return 0;
}
