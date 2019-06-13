#include "funciones.h"

//Definición de variables globales
pthread_mutex_t mutexBuffer;
pthread_mutex_t mutexVisibilidades; //Mutex del buffer - Solo una hebra puede estar leyendo o escribiendo  en el arreglo de
int discCant;
int discWidth;
int bFlag;
int tamanoBuffer;
Visibilidad ** datosVisibilidad;
//Función que lee una línea de un archivo de texto desde un archivo
//Entrada: Puntero al archivo del cuál se va a leer
//Salida: Cadena de char
char* readLine(FILE * file){
  int i = 0;
  //Asignación de memoria para las cadenas
  char* line = (char*)malloc(sizeof(char)*128);
  char* ch = (char*)malloc(sizeof(char)*64);

  int read;
  while((read = fread(ch, sizeof(char), 1, file)) == 1){
    if(ch[0] == 10){
      line[i] = '\0';
      i++;
      break;
    }
    else{
      line[i] = ch[0];
      i++;
    }
  }
  unsigned int lineLength = strlen(line);
  int j;
  char* fixedLine = (char*)malloc(sizeof(char)*128);
  for(j = 0; j < lineLength; j++){
    if(line[j] == 10 || line[j] == 13){
      fixedLine[j] = '\0';
    }
    else{
      fixedLine[j] = line[j];
    }
  }
  if(read == 0){
    fixedLine[0] = '\0';
  }
  free(ch);
  free(line);
  return fixedLine;
}

void writeFile(Visibilidad* visibilidad, char* nombreArchivo, int numDisco){
  FILE *fp;
  int i;
  char** str = (char**)malloc(4*sizeof(char*));
  for(i = 0; i < 4 ; i++){
    str[i] = (char*)malloc(20*sizeof(char));
  }
  str[0] = "Media real: ";
  str[1] = "Media imaginaria: ";
  str[2] = "Potencia: ";
  str[3] = "Ruido total: ";
  char numArr[512];
  sprintf(numArr, "%d", numDisco);
  char disco[] = "Disco ";
  char endDisc[] = ":\r\n";
  char endLine[] = "\r\n";
  fp = fopen(nombreArchivo, "a+b");
  fwrite(disco, sizeof(char) , strlen(disco), fp);
  fwrite(numArr, sizeof(char) , strlen(numArr), fp);
  fwrite(endDisc, sizeof(char), strlen(endDisc), fp);
    fwrite(str[0], sizeof(char), strlen(str[0]), fp);
    sprintf(numArr, "%lf", visibilidad->mediaReal);
    fwrite(numArr, sizeof(char), strlen(numArr), fp);
    fwrite(endLine, sizeof(char), strlen(endLine), fp);
    fwrite(str[1], sizeof(char), strlen(str[1]), fp);
    sprintf(numArr, "%lf", visibilidad->mediaImaginaria);
    fwrite(numArr, sizeof(char), strlen(numArr), fp);
    fwrite(endLine, sizeof(char), strlen(endLine), fp);
    fwrite(str[2], sizeof(char), strlen(str[2]), fp);
    sprintf(numArr, "%lf", visibilidad->potencia);
    fwrite(numArr, sizeof(char), strlen(numArr), fp);
    fwrite(endLine, sizeof(char), strlen(endLine), fp);
    fwrite(str[3], sizeof(char), strlen(str[3]), fp);
    sprintf(numArr, "%lf", visibilidad->ruidoTotal);
    fwrite(numArr, sizeof(char), strlen(numArr), fp);
    fwrite(endLine, sizeof(char), strlen(endLine), fp);
  fwrite(endLine, sizeof(char), strlen(endLine), fp);
  fclose(fp);
}

//Retorna el int correspondiente al disco que le corresponde la coordenada de entrada.
int checkDestination(double coordV, double coordU, int discWidth, int discCant){
    double distanceUV;
    double maxRadius = discWidth*discCant;
    distanceUV = sqrt(pow(coordV,2)+ pow(coordU,2));
    int disc = (distanceUV/discWidth);
    if(distanceUV > maxRadius){
       return discCant - 1;
    }
    else{
       return disc;
    }
}

void inicializarCharArray(char* array, int largo){
    int i;
    for(i = 0; i < largo; i++){
      array[i] = 0;
    }
}

int obtenerVisibilidadRecibida(char* visibilidad, int discWidth, int discCant){
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
    return checkDestination(data[0], data[1], discWidth, discCant); //BUSCO EL DISCO AL QUE PERTENECE LA INFO.
}

//Función ENTERSC: Buffer
//Entrada: Mutex buffer
//Salida: Void
void EnterSC(pthread_mutex_t * mutex)
{
    pthread_mutex_lock(mutex);
}

//Función EXIT: Buffer
//Entrada: Mutex buffer
//Salida: Void
void ExitSC(pthread_mutex_t * mutex)
{
    pthread_mutex_unlock(mutex);
}

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
    buffer->id = 0;
    buffer->cantidad = 0;
    buffer->estado = ABIERTO;
    buffer->full = 0;
    buffer->empty = 1;
    buffer->data = (char**)malloc(sizeof(char*)*tamanoBuffer);
    int i;
    for(i = 0; i < tamanoBuffer; i++){
      buffer->data[i] = (char*)malloc(sizeof(char)*128);
    }
    return buffer;
}

Visibilidad * inicializarVisibilidad(){
    Visibilidad * visibilidad = (Visibilidad*)malloc(sizeof(Visibilidad));
    visibilidad->mediaReal = 0;
    visibilidad->mediaImaginaria = 0;
    visibilidad->ruidoTotal = 0;
    visibilidad->potencia = 0;
    visibilidad->totalVisibilidades = 0;
    return visibilidad;
}

double* procesarDatosBuffer(double real, double imaginaria, double ruido, int totalVisibilidades, Buffer *b){
    int i;
    double *result = (double*)malloc(sizeof(double)*4);
    for(i = 0; i < b->cantidad; i++){
      double* data = obtenerDatosVisibilidad(b->data[i]);
      real = real + data[2];
      imaginaria = imaginaria + data[3];
      ruido = ruido + data[4];
    }
    totalVisibilidades = totalVisibilidades + b->cantidad;
    result[0] = real;
    result[1] = imaginaria;
    result[2] = ruido;
    result[3] = (double)totalVisibilidades;
    return result;
}

void almacenarDatos(double mediaReal, double mediaImaginaria, double ruido, double potencia, int totalVisibilidades, Visibilidad *visibilidad){
    visibilidad->mediaReal = mediaReal;
    visibilidad->mediaImaginaria = mediaImaginaria;
    visibilidad->ruidoTotal = ruido;
    visibilidad->potencia = potencia;
    visibilidad->totalVisibilidades = totalVisibilidades;
}

//Función que añade un dato a un buffer
//Entrada: Puntero al buffer que se añadirá un dato
//Salida: Vacío
void anadirDato(Buffer * b, char * line)
{
    strcpy(b->data[b->cantidad], line);
    b->cantidad = b->cantidad + 1;
    if(b->cantidad == tamanoBuffer){
      b->full = 1;
      b->empty = 0;
    }
    //printf("Cantidad de datos en el buffer: %i\n", b->cantidad);
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
    for(i = 0; i < tamanoBuffer; i++){
      b->data[i] = (char*)malloc(sizeof(char)*128);
    }
    b->cantidad = 0;
    b->full = 0;
    b->empty = 1;
}

//Función que limpia la memoria utilizada por los buffer.
//Entrada: Puntero al buffer que se añadirá un dato
//Salida: Vacío
//FINAL DE PROGRAMA.
void vaciarBufferSinReasignar(Buffer * b)
{
    int i;
    for(i=0; i<b->cantidad; i++)
    {
        free(b->data[i]);
    }
}

//Función debug que imprime los datos las visibilidades en pantalla
//Entrada: nada
//Salida: por pantalla
void mostrarVisibilidades()
{
    printf("\r\n");
    if(bFlag)
    {
        int i;
        int total = 0;
        for(i=0; i<discCant; i++)
        {
            printf("Soy la hebra %i, procese %i visibilidades.\n\r", i+1, datosVisibilidad[i]->totalVisibilidades);
            total = total + datosVisibilidad[i]->totalVisibilidades;
        }
        printf("Total de visibilidades procesadas por todos las hebras: %i\r\n", total);
    }
}
