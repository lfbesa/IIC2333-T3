// Libreria de input y output (para leer y escribir archivos o leer y escribir en consola)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "czfs_API.h"
//#include "linkedlist.h"

int main(int argc, char** argv)
{
  /* Revisamos que los parámetros sean correctos */
	if(argc != 2)
	{
		printf("Uso: %s <disco virtual>\nDonde\n", argv[0]);
    	printf("\t<disco virtual> es la ruta al archivo usado como disco virtual\n");
    	return 1;
	}

	

	
	cz_ls(argv[1]);
	printf("%d\n", cz_exists(argv[1], "texto.txt"));
	cz_mv(argv[1], "texto.txt", "text.txt");
	czFILE* file = cz_open(argv[1], "text.txt", 'w');
	char buf[102];
	printf("%ld\n", sizeof("hila"));
	cz_write(argv[1], file, "hila", sizeof("hila"));

	return 0;
}