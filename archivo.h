#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef Archivos_h
#define Archivos_h

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

#endif