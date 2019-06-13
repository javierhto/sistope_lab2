CodigoFuente = lab2.c
NombreEjecutable = lab2.exe

All:
	@echo "Compilando $(CodigoFuente)..."
	gcc $(CodigoFuente) -lm -pthread -o $(NombreEjecutable)
	@echo "OK!"
	@echo "Ejecutable: $(NombreEjecutable)"
	@echo "Para ejecutar: ./lab2.exe -i archivo_entrada -o archivo_salida -n número_discos -d ancho_discos -s tamaño_buffer -b"
	@echo "Prueba: ./lab2.exe -i prueba100.csv -o out.out -n 10 -d 20 -b -s 3"