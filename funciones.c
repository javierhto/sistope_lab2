//Función que lee una línea de un archivo de texto desde un archivo
//Entrada: Puntero al archivo del cuál se va a leer
//Salida: Cadena de char
char* readLine(FILE* file){
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
  if(line[i] == 10){
    line[i] = '\0';
  }
  if(read == 0){
    line[0] = '\0';
  }
  return line;
}

//Funnción que retorna el int correspondiente al disco que le corresponde la coordenada de entrada.
//Entrada: Coordenadas u,v, cantidad de discos y ancho de los discos
//Salida: Disco de destino
int checkDestination(double coordV, double coordU, int discWidth, int discCant){
    double distanceUV;
    double maxRadius = discWidth*discCant;
    distanceUV = sqrt(pow(coordV,2)+ pow(coordU,2)); //Se calcula la distancia con la fórmula entregada
    int disc = (distanceUV/discWidth);
    if(distanceUV > maxRadius){ //Esto hace que el último disco albergue todas las visibilidades que queden fuera del espectro máximo
       return discCant - 1;
    }
    else{
       return disc;
    }
}

//Función que inicializa un arreglo de char
//Entrada: Arreglo y largo del arreglo
//Salida: Nada, solo referencia
void inicializarCharArray(char* array, int largo){
    int i;
    for(i = 0; i < largo; i++){
      array[i] = 0;
    }
}

//Función que lee la visibilidad desde un cadena de datos
//Entrada: Cadena de datos, ancho del disco, cantidad de discos
//Salida: Visibilidad como entero
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

//Función que escribe en un archivo los datos procesados por un hijo
//Entrada: Arreglo de datos de un hijos, nombre del archivo de salida, número de disco actual
//Salida: Nada
void writeFile(double* informacionHijos, char* nombreArchivo, int numDisco){
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
  for(i = 0; i < 4; i++){
    fwrite(str[i], sizeof(char), strlen(str[i]), fp);
    sprintf(numArr, "%lf", informacionHijos[i]);
    fwrite(numArr, sizeof(char), strlen(numArr), fp);
    fwrite(endLine, sizeof(char), strlen(endLine), fp);
  }
  fwrite(endLine, sizeof(char), strlen(endLine), fp);
  fclose(fp);
}


////////////////////////////////////////

//Datoshijos:
    //0. media real
    //1. imaginaria
    //2. portencia
    //3. ruido
    //4. vis totales
    /*
      fs = fopen(fileIn, "r");
      int j;
      //Creando arreglo de Doubles que recibira la informacion de los hijos.
      double **dataHijos = (double**)malloc(sizeof(double*)*discCant);
      for(j = 0; j < discCant; j++){
        dataHijos[j] = (double*)malloc(sizeof(double)*512);
      }
      if(fs == NULL){
        printf("File %s does not exist.\r\n", fileIn);
        exit(0);
      }
      int count = 1;
      printf("Procesando linea: \r\n");
      while(!feof(fs)){
        char *line = readLine(fs); //Leemos cada linea del archivo en cuestion.
        if(line[0] == '\0'){
          //AQUI ES CUANDO SE AVISA A LOS HIJOS DE FIN
          //Y SE LES PIDE LOS DATOS CALCULADOS.
          //PLAN: RECIBIR LOS DATOS DE LOS HIJOS Y LUEGO ALMACENARLO EN UN ARCHIVO.
          char* endProgram = "FIN";
          for(j = 0; j < discCant; j++){
            write(pipesEscritura[j][ESCRITURA], endProgram, strlen(endProgram));
          }
          for(j = 0; j < discCant; j++){
            read(pipesLectura[j][LECTURA], dataHijos[j], 512);
          }
        }
        else{
          //AQUI SE LES ENTREGA LINEA A LINEA LOS DATOS DE ENTRADA.
          //A CADA HIJO QUE TENGAMOS.
          //PLAN: ENVIAR LINE AL HIJO SELECCIONADO EN DISC MEDIANTE PIPE.
          int disc = obtenerVisibilidadRecibida(line, discWidth, discCant);
          if(disc >= 0)
          {
            char* dataLength = (char*)malloc(sizeof(char)*10);
            sprintf(dataLength, "%li", strlen(line));
            write(pipesEscritura[disc][ESCRITURA], dataLength, 10);
            usleep(1500);
            write(pipesEscritura[disc][ESCRITURA], line, strlen(line));
          }
          //Esto permite hacer conocer al usuario que linea del archivo el programa esta leyendo.
          printf("\b\b\b\b\b\b\b\b\b");
          fflush(stdout);
          printf("%.7d", count);
          fflush(stdout);
          count = count + 1;
        }
        usleep(2000);
        free(line);
      }
      while ((wpid = wait(&status)) > 0); //FORZAMOS AL PADRE A ESPERAR POR TODOS SUS HIJOS
      //ESCRIBIMOS EN EL ARCHIVO LOS DATOS OBTENIDOS POR LOS HIJOS.
      for(i = 0; i < discCant; i++){
        writeFile(dataHijos[i], fileOut, i + 1);
      }
      //AQUI SE ENTREGA LOS RESULTADOS POR PANTALLA EN CASO DE QUE EL FLAG SEA VERDAD.
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
      for(i = 0; i < discCant; i++){
        free(dataHijos[i]);
      }
      free(dataHijos);
      //Ahora liberamos los pipes.
      for(i = 0; i < discCant; i++){
        free(pipesEscritura[i]);
        free(pipesLectura[i]);
      }
      free(pipesEscritura);
      free(pipesLectura);
      //Liberamos la memoria del resto.
      free(childs);
    */