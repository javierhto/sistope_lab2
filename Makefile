CodigoFuente = lab2.c
NombreEjecutable = lab2.exe

All:
	@echo "Compilando $(CodigoFuente)..."
	gcc $(CodigoFuente) -lm -pthread -o $(NombreEjecutable)
	@echo "OK!"
	@echo "Ejecutable: $(NombreEjecutable)"