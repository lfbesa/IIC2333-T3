// Libreria de input y output (para leer y escribir archivos o leer y escribir en consola)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "cz_API.h"
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

	cz_mount_disco(argv[1]);

	
	cz_ls();
	printf("%d\n", cz_exists("texto.txt"));
	cz_mv("texto.txt", "text.txt");
	czFILE* file = cz_open("ho1.txt", 'w');
	char buf[1110];
	
	cz_write(file, "chao", sizeof("chao"));

	cz_write(file, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin vel commodo sapien. In ultricies eget justo ut consectetur. Morbi in lorem non ipsum egestas auctor. Pellentesque nisl ligula, egestas vitae scelerisque sit amet, cursus vitae enim. In ante felis, cursus ac dui vel, posuere tristique velit. Aliquam viverra dolor sapien, vitae mattis enim bibendum ut. Vivamus sollicitudin, nisl sit amet faucibus efficitur, ipsum nisi cursus leo, sed suscipit felis nisi et turpis. Donec faucibus lectus lorem, at sodales enim fermentum et. Praesent vel pellentesque quam, et tempus erat. Vivamus imperdiet ante ut sapien iaculis, at ultricies magna pellentesque. Phasellus congue consectetur massa, id tincidunt augue. In accumsan pharetra tortor, ut venenatis nulla fermentum non. Nam eu commodo augue. Proin ac ultrices libero. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.Aliquam erat volutpat. In cursus pulvinar ligula, vitae lacinia magna condimentum et. Curabitur efficitur nunc facilisis turpis posuere, non laoreet arcu tristique. Pellentesque posuere.", sizeof("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin vel commodo sapien. In ultricies eget justo ut consectetur. Morbi in lorem non ipsum egestas auctor. Pellentesque nisl ligula, egestas vitae scelerisque sit amet, cursus vitae enim. In ante felis, cursus ac dui vel, posuere tristique velit. Aliquam viverra dolor sapien, vitae mattis enim bibendum ut. Vivamus sollicitudin, nisl sit amet faucibus efficitur, ipsum nisi cursus leo, sed suscipit felis nisi et turpis. Donec faucibus lectus lorem, at sodales enim fermentum et. Praesent vel pellentesque quam, et tempus erat. Vivamus imperdiet ante ut sapien iaculis, at ultricies magna pellentesque. Phasellus congue consectetur massa, id tincidunt augue. In accumsan pharetra tortor, ut venenatis nulla fermentum non. Nam eu commodo augue. Proin ac ultrices libero. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.Aliquam erat volutpat. In cursus pulvinar ligula, vitae lacinia magna condimentum et. Curabitur efficitur nunc facilisis turpis posuere, non laoreet arcu tristique. Pellentesque posuere."));
	int a = cz_read(file, buf, 1110);
	printf(" %s\n", buf);
	int b = cz_read(file, buf, 10);
	cz_rm("ho1.txt");
	cz_close(file);
	
	return 0;
}