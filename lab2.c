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
#include "funciones.c"

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
    Buffer * bufferLocal = buffer;

    while(bufferLocal->estado == ABIERTO)
    {
        EnterSC(&bufferLocal->mutex);
        while(bufferLocal->empty){
          pthread_cond_wait(&bufferLocal->notEmpty, &bufferLocal->mutex);
        }
        if(bufferLocal->cantidad == tamanoBuffer)
        {
            //Inicio sección crítica
            int i;
            for(i = 0; i < bufferLocal->cantidad; i++){
              double* data = obtenerDatosVisibilidad(bufferLocal->data[i]);
              real = real + data[2];
              imaginaria = imaginaria + data[3];
              ruido = ruido + data[4];
              totalVisibilidades = totalVisibilidades + 1;
            }
            vaciarBuffer(bufferLocal);
            //Fin sección crítica
        }
        pthread_cond_signal(&bufferLocal->notFull);
        ExitSC(&bufferLocal->mutex);
    }
    //Procesa el resto de datos del buffer que no fueron procesados
    EnterSC(&bufferLocal->mutex);
    int i;
    for(i = 0; i < bufferLocal->cantidad; i++){
      double* data = obtenerDatosVisibilidad(bufferLocal->data[i]);
      real = real + data[2];
      imaginaria = imaginaria + data[3];
      ruido = ruido + data[4];
      totalVisibilidades = totalVisibilidades + 1;
    }
    vaciarBuffer(bufferLocal);
    ExitSC(&bufferLocal->mutex);

    if(totalVisibilidades > 0){
      double mediaReal = ((double)1/(double)totalVisibilidades)*real;
      double mediaImaginaria = ((double)1/(double)totalVisibilidades)*imaginaria;
      double ruidoTotal = ruido;
      double potencia = sqrt(pow(mediaReal, 2) + pow(mediaImaginaria, 2));

      //INICIO ZONA SECCIÓN ESCRITURA EN 2 BUFFER
      EnterSC(&mutexVisibilidades);
      almacenarDatos(mediaReal, mediaImaginaria, ruidoTotal, potencia, totalVisibilidades, datosVisibilidad[bufferLocal->id]);
      ExitSC(&mutexVisibilidades);
      //FIN SECCIÓN CRÍTICA
    }
}


//////////////////////////////////////////////////////////////////////

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
    printf("Tamano buffer: %i\r\n", tamanoBuffer);

    //Se crea un arreglo de hebras del tamaño de la cantidad de discos
    pthread_t threads[discCant];
    //Se crean los mutex para las estructuras de buffer y visibilidades
    pthread_mutex_init(&mutexVisibilidades, NULL);
    pthread_mutex_init(&mutexBuffer, NULL);

    //pthread_mutex_t mutexAcumulador[discCant];

    //Se crea un arreglo de buffers en dónde se enviarán los datos a las hebras
    Buffer ** buffers = (Buffer**)malloc(sizeof(Buffer*)*discCant);
    datosVisibilidad = (Visibilidad**)malloc(sizeof(Visibilidad*)*discCant);
    //Se inicializa un mutex
    //pthread_mutex_init(&mutex, NULL);

    int i;
    for(i=0; i<discCant; i++) //Se crean tantas hebras como discos
    {
        buffers[i] = inicializarBuffer();
        buffers[i]->id = i;
        datosVisibilidad[i] = inicializarVisibilidad();
    }
    for(i=0; i<discCant; i++) //Se crean tantas hebras como discos
    {
        pthread_mutex_init(&buffers[i]->mutex, NULL);
        pthread_cond_init(&buffers[i]->notFull, NULL);
        pthread_cond_init(&buffers[i]->notEmpty, NULL);
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
    int count = 1;
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
           buffers[j]->empty = 0;
           buffers[j]->full = 1;
           buffers[j]->estado = CERRADO;
           pthread_cond_signal(&buffers[j]->notEmpty);
           //vaciarBufferSinReasignar(buffers[j]);
         }
         //PLAN: RECIBIR LOS DATOS DE LOS HIJOS Y LUEGO ALMACENARLO EN UN ARCHIVO.
       }
       else{
        //AQUI SE LES ENTREGA LINEA A LINEA LOS DATOS DE ENTRADA.
        //A CADA HIJO QUE TENGAMOS.
        //PLAN: ENVIAR LINE AL HIJO SELECCIONADO EN DISC MEDIANTE PIPE.
        int disc = obtenerVisibilidadRecibida(line, discWidth, discCant);
        if(disc >= 0)
        {
            EnterSC(&buffers[disc]->mutex);
            while(buffers[disc]->full){
              pthread_cond_wait(&buffers[disc]->notFull, &buffers[disc]->mutex);
            }
            anadirDato(buffers[disc], line);
            pthread_cond_signal(&buffers[disc]->notEmpty);
            ExitSC(&buffers[disc]->mutex);
        }
        //Esto permite hacer conocer al usuario que linea del archivo el programa esta leyendo.
        printf("\b\b\b\b\b\b\b\b\b");
        fflush(stdout);
        printf("%.7d", count);
        fflush(stdout);
        count = count + 1;
      }
        free(line);
    }
    fclose(fs);
    //FORZAMOS AL PADRE A ESPERAR POR TODOS SUS HIJOS
    //ESCRIBIMOS EN EL ARCHIVO LOS DATOS OBTENIDOS POR LOS HIJOS.
    for(i = 0; i < discCant; i++){
        //printf("Esperando a la hebra %i\n",i);
        pthread_join(threads[i], NULL);
    }
    mostrarVisibilidades();
    for(i = 0; i < discCant; i++){
      writeFile(datosVisibilidad[i], fileOut, i+1);
    }
    //Antes de finalizar el programa, liberamos la memoria.
    //Liberamos la memoria del arreglo doble de Double.

    printf("\r\n##### Fin de la ejecucion PADRE #####\r\n");
    return 0;
}

//Para ejecutar: ./lab2.exe -i prueba100.csv -o out.out -n 10 -d 20 -b -s 3
