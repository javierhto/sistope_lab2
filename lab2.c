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
  int hebra;
  double mediaReal;
  double mediaImaginaria;
  double ruidoTotal;
  double potencia;
}Visibilidad;

typedef struct VisibilidadHebra
{
  double real;
  double imaginaria;
  double ruido;
  int totalVisibilidades;
}VisibilidadHebra;

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

double* obtenerDatosVisibilidad(char* visibilidad){
    int i; //DECLARACION DE i PARA EL CICLO FOR.
    int j = 0; //DECLARACION DE j PARA POSICIONAR DOUBLE EN ARREGLO.
    int k = 0; //DECLARACION DE k PARA POSICIONAR CHAR EN ARREGLO.
    int len = strlen(visibilidad); //OBTENGO EL LARGO DEL STRING RECIBIDO.
    int arrayLength = 32;
    double* data = (double*)malloc(len*sizeof(double));
    char* temp = (char*)malloc(arrayLength*sizeof(char));
    inicializarCharArray(temp, arrayLength); //INICIALIZO EL ARREGLO PARA QUE NO TENGA BASURA.
    for(i = 0; i <= len; i++){
      if(i == len){ //SI LOS LARGOS SON IGUALES, SIGNIFICA QUE TERMINE DE PROCESAR LA INFORMACION.
        data[j] = atof(temp);
        free(temp);
      }
      else if(visibilidad[i] == 44){ //SI EN LA POSICION HAY UNA ',', SIGNIFICA QUE TERMINE DE LEER UN DATO.
        data[j] = atof(temp);
        j++;
        k = 0;
        free(temp); //LIBERO LA MEMORIA PARA EVITAR LEAKS.
        temp = (char*)malloc(32*sizeof(char)); //LE ASIGNO ESPACIO EN LA RAM NUEVAMENTE.
        inicializarCharArray(temp, arrayLength); //LE VUELVO A ASIGNAR CEROS PARA QUE NO HAYA BASURA.
      }
      else{
        temp[k] = visibilidad[i];
        k++;
      }
    }
    return data;
}

//Función que inicializa el buffer de un monitor
//Entrada: Nada - utiliza datos globales
//Salida: Puntero a buffer del tamaño instanciado
Buffer * inicializarBuffer()
{
    Buffer * buffer = (Buffer*)malloc(sizeof(Buffer));
    buffer->cantidad = 0;
    buffer->estado = ABIERTO;
    buffer->data = (char**)malloc(sizeof(char*)*tamanoBuffer);
    int i;
    for(i = 0; i < tamanoBuffer; i++){
      buffer->data[i] = (char*)malloc(sizeof(char)*128);
    }
    return buffer;
}

//Función que inicializa la informacion que almacena cada hebra.
//Entrada: Nada - utiliza datos globales
//Salida: Puntero a la estructura del tamaño instanciado
VisibilidadHebra * inicializarDataHebras()
{
    VisibilidadHebra * dataHebra = (VisibilidadHebra*)malloc(sizeof(VisibilidadHebra));
    dataHebra->real = 0;
    dataHebra->imaginaria = 0;
    dataHebra->ruido = 0;
    dataHebra->totalVisibilidades = 0;
    return dataHebra;
}

void procesarDatosBuffer(double real, double imaginaria, double ruido, double totalVisibilidades, Buffer *b){
    int i;
    for(i = 0; i < b->cantidad; i++){
      double* data = obtenerDatosVisibilidad(b->data[i]);
      real = real + data[2];
      imaginaria = imaginaria + data[3];
      ruido = ruido + data[4];
      totalVisibilidades++;
    }
}

//Función que añade un dato a un buffer
//Entrada: Puntero al buffer que se añadirá un dato
//Salida: Vacío
void anadirDato(Buffer * b, char * line)
{
    strcpy(b->data[b->cantidad], line);
    b->cantidad = b->cantidad + 1;
    if(b->cantidad == tamanoBuffer){
      b->estado = CERRADO;
    }
}

//Función que añade un dato a un buffer
//Entrada: Puntero al buffer que se añadirá un dato
//Salida: Vacío
//SECCIÓN CRÍTICA
void vaciarBuffer(Buffer * b)
{
    int i;
    for(i=0; i<b->cantidad; i++)
    {
        free(b->data[i]);
    }
    b->cantidad = 0;
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
    //Casteo de los datos para la hebra.
    double real = 0;
    double imaginaria = 0;
    double ruido = 0;
    int totalVisibilidades = 0;

    //Casteo del buffer entregado
    Buffer * bufferLocal = (Buffer*)buffer;

    while(bufferLocal->estado == ABIERTO)
    {
        if(bufferLocal->cantidad == tamanoBuffer)
        {
            procesarDatosBuffer(real, imaginaria, ruido, totalVisibilidades, bufferLocal);
            vaciarBuffer(bufferLocal);
        }
    }
    printf("Fin \r\n");
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

    //DEBUG
    printf("Iniciando procesamiento con %i discos...\r\n", discCant);

    //Se crea un arreglo de hebras del tamaño de la cantidad de discos
    pthread_t threads[discCant];

    //Se crea un arreglo de buffers en dónde se enviarán los datos a las hebras
    Buffer ** buffers = (Buffer**)malloc(sizeof(Buffer*)*discCant);
    VisibilidadHebra ** informacionhebras = (VisibilidadHebra**)malloc(sizeof(VisibilidadHebra*)*discCant);

    //Se inicializa un mutex
    //pthread_mutex_init(&mutex, NULL);

    int i;
    for(i=0; i<discCant; i++) //Se crean tantas hebras como discos
    {
        buffers[i] = inicializarBuffer();
        informacionhebras[i] = inicializarDataHebras();
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
    printf("Procesando linea: \r\n");
    while(!feof(fs)){
       char * line = readLine(fs); //Leemos cada linea del archivo en cuestion.
       if(line[0] == '\0'){
         //AQUI ES CUANDO SE AVISA A LOS HIJOS DE FIN CERRANDO LOS BUFFERS
         //Y SE LES PIDE LOS DATOS CALCULADOS.
         for(j = 0; j < discCant; j++){
           buffers[j]->estado = CERRADO;
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
       printf("%s\n\r", line);
       free(line);
    }
    printf("FIN\r\n");
    fclose(fs);

    //FORZAMOS AL PADRE A ESPERAR POR TODOS SUS HIJOS
    //ESCRIBIMOS EN EL ARCHIVO LOS DATOS OBTENIDOS POR LOS HIJOS.
    for(i = 0; i < discCant; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Real: %f, img: %f, ruido: %f\r\n", informacionhebras[9]->real, informacionhebras[9]->imaginaria, informacionhebras[9]->ruido);
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

    printf("\r\n##### Fin de la ejecucion PADRE #####\r\n");
    return 0;
}

//Para ejecutar: ./lab2.exe -i prueba100.csv -o out.out -n 10 -d 20 -b -s 3
