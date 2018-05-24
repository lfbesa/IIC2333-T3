// Libreria de input y output (para leer y escribir archivos o leer y escribir en consola)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "czfs_API.h"
//#include "linkedlist.h"

int main(int argc, char** argv)
{
  /* Revisamos que los parámetros sean correctos */
	if(argc != 3)
	{
		printf("Uso: %s <disco virtual>\nDonde\n", argv[0]);
    	printf("\t<disco virtual> es la ruta al archivo usado como disco virtual\n");
    	return 1;
	}

	

	
	cz_ls(argv[1]);
	printf("%d\n", cz_exists(argv[1], "texto.txt"));
	cz_mv(argv[1], "texto.txt", "text.txt");
	czFILE* file = cz_open(argv[1], argv[2], 'w');
	char buf[1024];
	cz_write(argv[1], file, "hila", sizeof("hila"));
	cz_write(argv[1], file, "chao", sizeof("chao"));
	cz_read(argv[1], file, buf, 11);
	printf("%s\n", buf);
	for (int j=0;j<9;j++){
		if (buf[j]=='\0'){
			buf[j] = 32;
		}
	}
	printf("%s\n", buf);
	cz_close(argv[1], file);
	unsigned char byte = 49;// Read from file
	unsigned char bits[8];

	for (int i = 0; i < 8; i++) {
	    bits[i] = (byte >> i) & 1;
	}
	 // For debug purposes, lets print the received data
	int k=0;
	for (int i = 7; i >=0; i--) {
		int power =  pow(2,i);
		k += bits[i]*power;
		printf("%d", bits[i]);
	}
	printf("\n");
	printf("%d\n", k);
	printf("\n");
	return 0;
}