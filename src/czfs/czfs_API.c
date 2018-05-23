#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "czfs_API.h"

#pragma pack(1)
typedef struct TIMESTAMP_32_B_S
   {
   uint8_t   minutes    : 6; // 0-60 (0-63 max)
   uint8_t   hours24    : 5; // 0-23 (0-31 max)
   uint8_t   dayOfMonth : 5; // 1-31 (0-31 max)
   uint8_t   month      : 4; // 1-12 (0-15 max)
   uint16_t  year       : 12; // Epoch start: 2014, Range: 2014 thru 4061
   } TIMESTAMP_32_B_T;
#pragma pack()


czFILE* cz_open(char *disco, char* filename, char mode){
	printf("%s\n", "---- open -----");
	FILE *disk;
	disk = fopen(disco,"r+b");
	int existe = -1;

	unsigned char valid[1];
	char name[11];
	unsigned char indic[4];

	fseek(disk, 0, SEEK_SET);

	unsigned char buffer[16];
	
	fread(buffer,16,1,disk);


	memcpy( valid, &buffer[0], 1);
	memcpy( name, &buffer[1], 11);
	memcpy( indic, &buffer[12], 4);


	int donde_escribir_nombre=-1;
	
	while ( ftell(disk)<=1024){
		if (*valid == 1){
			if (strcmp(name,filename) == 0){
				if (mode=='r'){
					existe = 1;
					break; 
				}
				else if (mode=='w'){
					fprintf(stderr,"cz_open: %s ya existe por lo que no se puede abrir en 'w' \n", filename);
					fclose(disk); 
					return NULL;
				}
			}
		}
		else if (*valid == 0 && donde_escribir_nombre < 0){
			donde_escribir_nombre = ftell(disk) - 16;
		}
			
		fread(buffer,16,1,disk);

		memcpy( valid, &buffer[0], 1);
		memcpy( name, &buffer[1], 11);
		memcpy( indic, &buffer[12], 4);
	}

	if (mode == 'r'){
		if (existe==-1)
		{
			fprintf(stderr,"cz_open: %s no existe \n", filename);
			fclose(disk); 
			return NULL;
		}
		czFILE* file = calloc(1,sizeof(czFILE));
		file->dondevoy=0;
		file->indice = (indic[2]<<8)+indic[3];
		file->bloque = 0;
		file->mode = 0;
		memcpy( file->nombre, name, 11);
		fseek(disk, 1024*(file->indice), SEEK_SET);
		unsigned char tam[4];
		unsigned char bloq_ind[1024];
		fread(bloq_ind,1024,1,disk);
		memcpy(tam, &bloq_ind[0], 4);
		file->tamano = (tam[2]<<24)+(tam[2]<<16)+(tam[2]<<8)+tam[3];
		printf("%d\n",(tam[2]<<24)+(tam[2]<<16)+(tam[2]<<8)+tam[3]);
		fclose(disk); 
		return file;
	} else if (mode== 'w'){
		fseek(disk, donde_escribir_nombre, SEEK_SET);
		//Crear archivo nuevo en disco
		char new_name[11];
		for (int i = 0;i<11;i++){
			new_name[i] = 0;
		}
		int large = sizeof(filename);
		memcpy( new_name, filename, large);
		printf("%s\n", new_name);
		int valido = 1;
		fwrite(&valido,1,1, disk);
		fwrite(new_name, 11, 1, disk);
		long int current = ftell(disk);

		//Buscar lugar para su bloque indice y inicializarlo
		fseek(disk, 1024, SEEK_SET);
		unsigned char bitmaps[8192];
		fread(bitmaps, sizeof(unsigned char), 8192, disk);
		fseek(disk, 1024, SEEK_SET);
		int bloque;
		for (int n=0;n<8192;n++){
			if (bitmaps[n]!=255){
				int numero = bitmaps[n]+1;
				bool encontrado=true;
				int bit=0;
				while (encontrado){
					if((bitmaps[n] & 1) == 0){
						encontrado=false;
    					printf("EVEN!\n");
    				}
    				bitmaps[n] = (bitmaps[n] -1)/2;
    				bit++;
				}
				bloque = 8*n + (8-bit);
				printf("%d\n", bloque); 
				bitmaps[n] = numero;
				break;
			}
		}
		fseek(disk, 1024, SEEK_SET);
		fwrite(bitmaps, 8192, 1, disk);

		fseek(disk, 1024*bloque, SEEK_SET);
		time_t ltime; /* calendar time */
	    ltime=time(NULL); /* get current cal time */
	    TIMESTAMP_32_B_T timestamp;
	    timestamp.hours24 = (localtime(&ltime))->tm_hour;
	    timestamp.dayOfMonth = (localtime(&ltime))->tm_hour;
	    timestamp.month = (localtime(&ltime))->tm_mon;
	    timestamp.year = (localtime(&ltime))->tm_year;
	    timestamp.minutes = (localtime(&ltime))->tm_min;
	    int metadata = 0;
	    fwrite(&metadata,sizeof(int),1, disk);
	    fwrite(&timestamp,sizeof(TIMESTAMP_32_B_T),1, disk);
	    fwrite(&timestamp,sizeof(TIMESTAMP_32_B_T),1, disk);


   		//escribir bloque indice y retornar 
		fseek(disk, current, SEEK_SET);
		unsigned char bloque_char[4];
		bloque_char[0] = (bloque >> 24) & 0xFF;
		bloque_char[1] = (bloque >> 16) & 0xFF;
		bloque_char[2] = (bloque >> 8) & 0xFF;
		bloque_char[3] = bloque & 0xFF;
		fwrite(bloque_char, 4,1,disk);
		czFILE* file = calloc(1,sizeof(czFILE));
		file->dondevoy=1023;
		file->indice = bloque;
		file->bloque = 0;
		file->mode = 1;
		file->tamano = 0;
		memcpy( file->nombre, new_name, 11);
		fclose(disk);
		return file; 
	}
	else {
		fprintf(stderr,"cz_open: modo de abrir '%c' no existe \n", mode);
	}
	fclose(disk); 
	return NULL;
}

int cz_exists(char *disco, char* filename){
	printf("%s\n", "---- exists -----");
	FILE *disk;
	disk = fopen(disco,"rb");  

	unsigned char valid[1];
	char name[11];
	unsigned char indice[4];

	fseek(disk, 0, SEEK_SET);

	unsigned char buffer[16];
	
	fread(buffer,16,1,disk);


	memcpy( valid, &buffer[0], 1);
	memcpy( name, &buffer[1], 11);
	memcpy( indice, &buffer[12], 4);
	
	
	while ( (ftell(disk)<=1024)){
		if (*valid == 1){
			if (strcmp(name,filename) == 0){
				return 1;
			}
		}

		fread(buffer,16,1,disk);


		memcpy( valid, &buffer[0], 1);
		memcpy( name, &buffer[1], 11);
		memcpy( indice, &buffer[12], 4);
		
	}
	fclose(disk); 
	return 0;
}

int cz_read(char *disco, czFILE* file_desc, void* buffer, int nbytes){
	if (file_desc == NULL){
		char algo[nbytes];
		for (int n=0; n<nbytes;n++){
			algo[n] = 0;
		}
		memcpy(buffer, algo, nbytes);
		return -1;
	}
	printf("%s\n", "---- read -----");
	FILE *disk;
	disk = fopen(disco,"rb");
	fseek(disk, 1024*(file_desc->indice), SEEK_SET);
	unsigned char bloq_ind[1024];
	fread(bloq_ind,1024,1,disk);
	if (file_desc->mode){
		fprintf(stderr,"cz_read: %s modo de abierto en  'w' \n", file_desc->nombre);
		return -1;
	}

	//termina archivo
	if (((file_desc->bloque)*1024 + (file_desc->dondevoy)) >= (file_desc->tamano)){
		char algo[nbytes];
		printf("%s\n", "fin archivo");
		for (int n=0; n<nbytes;n++){
			algo[n] = 0;
		}
		memcpy(buffer, algo, nbytes);
		fclose(disk);
		return 0;
	}
	// no queda sufic archivo para nbytes
	else if (((file_desc->bloque)*1024 + (file_desc->dondevoy) + nbytes)>= (file_desc->tamano)){
		printf("%s\n","no sufic" );
		int a_leer = file_desc->tamano - ((file_desc->bloque)*1024 + (file_desc->dondevoy));
		if (file_desc->bloque > 251){
			int bloq_indirec = (bloq_ind[12 + 252*4 + 2]<<8)+bloq_ind[12 + 252*4+ 3];

			fseek(disk, 1024*bloq_indirec, SEEK_SET);
			fseek(disk,((file_desc->bloque) - 252)*4  , SEEK_CUR);
			unsigned char bloq[4];
			fread(bloq, 4,1,disk);
			int bloque_a_leer = (bloq[2]<<8)+bloq[3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer, a_leer, 1, disk);
			file_desc->dondevoy += a_leer;
			fclose(disk);
			return a_leer;


		}
		else {
			int bloque_a_leer = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer, a_leer, 1, disk);
			file_desc->dondevoy += a_leer;
			fclose(disk);
			return a_leer;
		}
		fclose(disk);
		return a_leer;

	}
	//Cambio de bloque (incluye bloque indirecto)
	else if (file_desc->dondevoy + nbytes>=1023){
		int leer_primero = 1024 - file_desc->dondevoy;
		int leer_segundo = nbytes - leer_primero;
		int bloq_indirec = (bloq_ind[12 + 252*4 + 2]<<8)+bloq_ind[12 + 252*4+ 3];
		char buffer1[leer_primero];
		char buffer2[leer_segundo];

		if (file_desc->bloque > 251){

			fseek(disk, 1024*bloq_indirec, SEEK_SET);
			fseek(disk,((file_desc->bloque) - 252)*4  , SEEK_CUR);
			unsigned char bloq[4];
			fread(bloq, 4,1,disk);
			int bloque_a_leer = (bloq[2]<<8)+bloq[3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer1, leer_primero, 1, disk);


		}
		else {
			int bloque_a_leer = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer1, leer_primero, 1, disk);
		}
		file_desc->dondevoy = 0;
		file_desc->bloque += 1;
		if (file_desc->bloque > 251){

			fseek(disk, 1024*bloq_indirec, SEEK_SET);
			fseek(disk,((file_desc->bloque) - 252)*4  , SEEK_CUR);
			unsigned char bloq[4];
			fread(bloq, 4,1,disk);
			int bloque_a_leer = (bloq[2]<<8)+bloq[3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer2, leer_segundo, 1, disk);


		}
		else {
			int bloque_a_leer = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer2, leer_segundo, 1, disk);
		}
		strcpy(buffer, buffer1);
	    strcat(buffer, buffer2);
		fclose(disk);
		return nbytes;
		

	}
	//lecura normal
	else {
		if (file_desc->bloque > 251){
			int bloq_indirec = (bloq_ind[12 + 252*4 + 2]<<8)+bloq_ind[12 + 252*4+ 3];

			fseek(disk, 1024*bloq_indirec, SEEK_SET);
			fseek(disk,((file_desc->bloque) - 252)*4  , SEEK_CUR);
			unsigned char bloq[4];
			fread(bloq, 4,1,disk);
			int bloque_a_leer = (bloq[2]<<8)+bloq[3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer, nbytes, 1, disk);
			file_desc->dondevoy += nbytes;
			fclose(disk);
			return nbytes;


		}
		else {
			int bloque_a_leer = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer, nbytes, 1, disk);
			file_desc->dondevoy += nbytes;
			fclose(disk);
			return nbytes;
		}
	}

	fclose(disk);
	return -1;
}

int cz_write(char *disco, czFILE* file_desc, void* buffer, int nbytes){
	if (file_desc == NULL){
		
		return -1;
	}
	printf("%s\n", "---- write -----");
	FILE *disk;
	disk = fopen(disco,"r+b");
	if (!(file_desc->mode)){
		fprintf(stderr,"cz_write: %s abierto en modo 'r' \n", file_desc->nombre);
		return -1;
	}
	fseek(disk, 1024*(file_desc->indice), SEEK_SET);
	unsigned char bloq_ind[1024];
	fread(bloq_ind,1024,1,disk);
	int bloq_indirec = (bloq_ind[12 + 252*4 + 2]<<8)+bloq_ind[12 + 252*4+ 3];

	unsigned char times[4];
	memcpy(times, bloq_ind, 4);
	if (file_desc->tamano==0){
		printf("%s\n", times);
		printf("%d\n", bloq_indirec);
	}
	else if (file_desc->dondevoy + nbytes > 1023){
		int primero = 1024 - file_desc->dondevoy;
		int segundo = nbytes - primero;
		unsigned char escrib1[primero];
		unsigned char escrib2[segundo];
		memcpy( escrib1, &buffer[0], primero);
		memcpy( escrib2, &buffer[primero], segundo);

		//falta esto

		fclose(disk);
		return 1;
	}
	else {
		if (file_desc->bloque > 251){
			fseek(disk, 1024*bloq_indirec, SEEK_SET);
			fseek(disk, ((file_desc->bloque) - 252)*4  , SEEK_CUR);
			unsigned char bloq[4];
			fread(bloq, 4,1,disk);
			int bloque_a_escribir = (bloq[2]<<8)+bloq[3];
			fseek(disk, 1024*bloque_a_escribir, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fwrite(buffer, nbytes, 1, disk);
		}
		else {
			int bloque_a_escribir = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_escribir, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fwrite(buffer, nbytes, 1, disk);
		}
		file_desc->dondevoy += nbytes;
		file_desc->tamano += nbytes;
		fclose(disk);
		return nbytes;
	}
	return 1;
}

int cz_close(char *disco, czFILE* file_desc){
	printf("%s\n", "---- close -----");

	// Escribir el TAMAÑO  FALTAA
	int nuevo_tam = file_desc->tamano;
	FILE *disk;
	disk = fopen(disco,"r+b");
	fseek(disk, 1024*(file_desc->indice), SEEK_SET);
	fwrite(&nuevo_tam, sizeof(int),1, disk);
	fclose(disk); 

	free(file_desc);

	return 0;
}

int cz_mv(char *disco, char* orig, char *dest){
	printf("%s\n", "---- mv -----");
	FILE *disk;
	disk = fopen(disco,"r+b");
	int donde = -1;  

	unsigned char valid[1];
	char name[11];
	unsigned char indice[4];

	fseek(disk, 0, SEEK_SET);

	unsigned char buffer[16];
	
	fread(buffer,16,1,disk);


	memcpy( valid, &buffer[0], 1);
	memcpy( name, &buffer[1], 11);
	memcpy( indice, &buffer[12], 4);
	
	
	while ( (ftell(disk)<=1024)){
		if (*valid == 1){
			if (strcmp(name,dest) == 0){
				fprintf(stderr,"cz_mv: %s ya existe \n", dest);
				return 1;
			}
			else if (strcmp(name,orig) == 0){
				donde = ftell(disk) - 16;
			}
		}


		fread(buffer,16,1,disk);


		memcpy( valid, &buffer[0], 1);
		memcpy( name, &buffer[1], 11);
		memcpy( indice, &buffer[12], 4);
	}
	if (donde==-1)
	{
		fprintf(stderr,"cz_mv: %s no existe \n", orig);
		return 2;
	}
	char new_name[11];
	for (int i = 0;i<11;i++){
		new_name[i] = 0;
	}
	int large = sizeof(dest);
	memcpy( new_name, dest, large);
	fseek(disk, donde+1, SEEK_SET);
	fwrite(new_name, 11, 1, disk);

	fclose(disk); 
	return 1;
}

int cz_cp(char *disco, char* orig, char* dest){
	printf("%s\n", "---- cp -----");
	return 1;
}

int cz_rm(char *disco, char* filename){
	printf("%s\n", "---- rm -----");
	return 1;
}


void cz_ls(char *disco){
	printf("%s\n", "---- ls -----");
	FILE *disk;
	disk = fopen(disco,"rb");  // r for read, b for binary

	unsigned char valid[1];
	unsigned char name[11];
	unsigned char indice[4];

	fseek(disk, 0, SEEK_SET);

	unsigned char buffer[16];
	
	fread(buffer,16,1,disk);

	memcpy( valid, &buffer[0], 1);
	memcpy( name, &buffer[1], 11);
	memcpy( indice, &buffer[12], 4);
	
	
	
	while ( (ftell(disk)<=1024)){
		if (*valid == 1){
			for (int i=0; i<1;i++){
				printf("valid %u \n", valid[i]);
			}
			printf("ftell %ld\n", ftell(disk));
		
			printf("%s\n", name);

			int number = (indice[2]<<8)+indice[3];
			printf("numero %d \n", number);


		}
		fread(buffer,16,1,disk);


		memcpy( valid, &buffer[0], 1);
		memcpy( name, &buffer[1], 11);
		memcpy( indice, &buffer[12], 4);
	}

	fclose(disk);  


	
}
