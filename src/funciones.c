#include "funciones.h"

//Definición de variables globales
pthread_mutex_t mutexBuffer;
pthread_mutex_t mutexVisibilidades; //Mutex del buffer - Solo una hebra puede estar leyendo o escribiendo  en el arreglo de
int discCant;
int discWidth;
int bFlag;
int tamanoBuffer;
Visibilidad ** datosVisibilidad;

//Entrada: monitor buffer.
//funcionalidad: Función que inciializa la ejecuión de una hebra
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
            //UNA VEZ PROCESADO EL BUFFER, SACAMOS LOS RESULTADOs PARCIALES.
            bufferLocal->mediaReal = ((double)1/(double)totalVisibilidades)*real;
            bufferLocal->mediaImaginaria = ((double)1/(double)totalVisibilidades)*imaginaria;
            bufferLocal->ruidoTotal = ruido;
            bufferLocal->potencia = sqrt(pow(bufferLocal->mediaReal, 2) + pow(bufferLocal->mediaImaginaria, 2));
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
      bufferLocal->mediaReal = ((double)1/(double)totalVisibilidades)*real;
      bufferLocal->mediaImaginaria = ((double)1/(double)totalVisibilidades)*imaginaria;
      bufferLocal->ruidoTotal = ruido;
      bufferLocal->potencia = sqrt(pow(bufferLocal->mediaReal, 2) + pow(bufferLocal->mediaImaginaria, 2));

      //INICIO ZONA SECCIÓN ESCRITURA EN 2 BUFFER
      EnterSC(&mutexVisibilidades);
      almacenarDatos(bufferLocal, totalVisibilidades, datosVisibilidad[bufferLocal->id]);
      ExitSC(&mutexVisibilidades);
      //FIN SECCIÓN CRÍTICA
    }
}

//Entrada: Puntero al archivo del cuál se va a leer
//funcionalidad: Función que lee una línea de un archivo de texto desde un archivo
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

//Entrada: Visibilidad: puntero a la estructura comun de datos, nombreArchivo: nombre del archivo a guardar, numDisco: numero del disco a guardar.
//funcionalidad: Función que almacena en un archivo de texto los datos obtenidos por las visibilidades.
//Salida: Vacio.
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

//Entrada: coordV: Coordenada v de la visibilidad, coordU: Coordenada U de la visibilidad, discWidth: Ancho de los discos, discCant: Cantidad de discos.
//Funcionalidad: Retorna el int correspondiente al disco que le corresponde la coordenada de entrada.
//Salida: el numero de disco al cual pertenecen las coordenadas.
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

//ENTRADAS: ARRAY: ARREGLO A DEJAR EN 0, LARGO: LARGO DEL ARREGLO.
//FUNCIONALIDAD: FUNCION QUE INICIALIZA EN 0 UN ARREGLO BASADO EN UN LARGO.
//SALIDAS: VACIO.
void inicializarCharArray(char* array, int largo){
    int i;
    for(i = 0; i < largo; i++){
      array[i] = 0;
    }
}

//ENTRADAS: VISIBILIDAD: LINEA DEL ARCHIVO DE TEXTO, discWidth: ANCHO DEL DISCO, DiscCant: CANTIDAD DE DISCOS.
//FUNCIONALIDAD: FUNCION QUE OBTIENE LOS DATOS COMO DOUBLE DE LAS VISIBILIDADES RECIBIDAS.
//SALIDA: RETORNO A FUNCION QUE DETERMINA EL DISCO DE LA VISIBILIDAD.
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

//Entrada: Mutex buffer
//Funcionalidad: Función ENTERSC: Buffer
//Salida: Void
void EnterSC(pthread_mutex_t * mutex)
{
    pthread_mutex_lock(mutex);
}

//Entrada: Mutex buffer
//Funcionalidad: Función EXIT: Buffer
//Salida: Void
void ExitSC(pthread_mutex_t * mutex)
{
    pthread_mutex_unlock(mutex);
}

//ENTRADAS: VISIBILIDAD: LINEA DEL ARCHIVO DE TEXTO.
//FUNCIONALIDAD: FUNCION QUE OBTIENE LOS DATOS COMO DOUBLE DE LAS VISIBILIDADES RECIBIDAS.
//SALIDA: ARREGLO DE DOUBLE CON LOS DATOS DE LA Visibilidad.
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

//Entrada: Nada - utiliza datos globales
//Funcionalidad: Función que inicializa el buffer de un monitor
//Salida: Puntero a buffer del tamaño instanciado
Buffer * inicializarBuffer()
{
    Buffer * buffer = (Buffer*)malloc(sizeof(Buffer));
    buffer->id = 0;
    buffer->cantidad = 0;
    buffer->mediaReal = 0;
    buffer->mediaImaginaria = 0;
    buffer->ruidoTotal = 0;
    buffer->potencia = 0;
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

//Entrada: Nada - utiliza datos globales
//Funcionalidad: Función que inicializa la estructura comun de datos.
//Salida: Puntero a la estructura comun de datos.
Visibilidad * inicializarVisibilidad(){
    Visibilidad * visibilidad = (Visibilidad*)malloc(sizeof(Visibilidad));
    visibilidad->mediaReal = 0;
    visibilidad->mediaImaginaria = 0;
    visibilidad->ruidoTotal = 0;
    visibilidad->potencia = 0;
    visibilidad->totalVisibilidades = 0;
    return visibilidad;
}

//ENTRADA: B: MONITOR BUFFER, totalVisibilidades: CANTIDAD TOTAL DE VISIBILIDADES OBTENIDAS. VISIBILIDAD: PUNTERO A ESTA ESTRUCTURA.
//FUNCIONALIDAD: FUNCION QUE ALMACENA LOS DATOS DE LA ESTRUCTURA DE DATOS EN COMUN.
//SALIDA: VACIO.
void almacenarDatos(Buffer *b, int totalVisibilidades, Visibilidad *visibilidad){
    visibilidad->mediaReal = b->mediaReal;
    visibilidad->mediaImaginaria = b->mediaImaginaria;
    visibilidad->ruidoTotal = b->ruidoTotal;
    visibilidad->potencia = b->potencia;
    visibilidad->totalVisibilidades = totalVisibilidades;
}

//Entrada: Puntero al buffer que se añadirá un dato
//Funcionalidad: Función que añade un dato a un buffer
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

//Entrada: Puntero al buffer que se añadirá un dato
//Funcionalidad: Función que añade un dato a un buffer
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

//Entrada: Puntero al buffer que se añadirá un dato
//Funcionalidad: Función que limpia la memoria utilizada por los buffer.
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

//Entrada: nada
//Funcionalidad: Función que imprime los datos las visibilidades procesadas por cada hebra en pantalla
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
