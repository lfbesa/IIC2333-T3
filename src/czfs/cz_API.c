#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#include "cz_API.h"


#define MIN(x,y) ((x<y)?x:y)
char discos[100];
void cz_mount_disco(char *virtua){
	memcpy(discos, virtua, strlen(virtua));
}

void clean_buffer(char *buffer, int donde){
	for (int j=0;j<donde - 2;j++){
		if (buffer[j]=='\0'){
			buffer[j] = 32;
		}
	}
	buffer[donde-1]='\0';
}

czFILE* cz_open(char* filename, char mode){
	FILE *disk;
	disk = fopen(discos,"r+b");
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
		memcpy(tam, bloq_ind, 4);
		int x = (tam[0] << 24) | (tam[1] << 16) | (tam[2] << 8) | tam[3];
		file->tamano = x;
		unsigned char dato[4];
		memcpy(dato, &bloq_ind[12], 4);
		int* pInt2 = (int*)dato;
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
				unsigned char bits[8];
				int bit=0;
				for (int i = 0; i < 8; i++) {
				    bits[i] = (bitmaps[n] >> i) & 1;
				}
				for (int i = 7; i >=0; i--) {
					if (bits[i]==0){
						bit=i;
						bits[i] = 1;
						break;
					}
				}
				int numero=0;
				for (int i = 7; i >=0; i--) {
					int power =  pow(2,i);
					numero += bits[i]*power;
				}
				bloque = 8*n + (7-bit);
				bitmaps[n] = numero;
				break;
			}
		}
		fseek(disk, 1024, SEEK_SET);
		fwrite(bitmaps, 8192, 1, disk);

		fseek(disk, 1024*bloque, SEEK_SET);
		time_t ltime; /* calendar time */
	    ltime=time(NULL); /* get current cal time */
	    // TIMESTAMP_32_B_T timestamp;
	    // timestamp.hours24 = (localtime(&ltime))->tm_hour;
	    // timestamp.dayOfMonth = (localtime(&ltime))->tm_hour;
	    // timestamp.month = (localtime(&ltime))->tm_mon;
	    // timestamp.year = (localtime(&ltime))->tm_year;
	    // timestamp.minutes = (localtime(&ltime))->tm_min;
	    int metadata = 0;
	    fwrite(&metadata,sizeof(int),1, disk);
	    fwrite(&ltime,sizeof(ltime),1, disk);
	    fwrite(&ltime, sizeof(ltime),1, disk);


   		//escribir bloque indice y retornar 
		fseek(disk, current, SEEK_SET);
		unsigned char bloque_char[4];
		bloque_char[0] = (bloque >> 24) & 0xFF;
		bloque_char[1] = (bloque >> 16) & 0xFF;
		bloque_char[2] = (bloque >> 8) & 0xFF;
		bloque_char[3] = bloque & 0xFF;
		fwrite(bloque_char, 4,1,disk);
		czFILE* file = calloc(1,sizeof(czFILE));
		file->dondevoy=0;
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

int cz_exists(char* filename){
	FILE *disk;
	disk = fopen(discos,"rb");  

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

int cz_read(czFILE* file_desc, void* buffer, int nbytes){
	if (!file_desc){
		char algo[nbytes];
		for (int n=0; n<nbytes;n++){
			algo[n] = 0;
		}
		memcpy(buffer, algo, nbytes);
		fprintf(stderr,"cz_read: archivo no abierto, FILE* NULL \n");
		return -1;
	}
	FILE *disk;
	disk = fopen(discos,"rb");
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
		for (int n=0; n<nbytes;n++){
			algo[n] = 0;
		}
		memcpy(buffer, algo, nbytes);
		fclose(disk);
		return 0;
	}
	
	//Cambio de bloque (incluye bloque indirecto)
	else if (file_desc->dondevoy + nbytes>=1023){
		int numero_a_leer = nbytes;
		int leer_primero = 1024 - file_desc->dondevoy;
		// no queda sufic archivo para nbytes
		if (((file_desc->bloque)*1024 + (file_desc->dondevoy) + nbytes)>= (file_desc->tamano)){
			numero_a_leer = file_desc->tamano - ((file_desc->bloque)*1024 + (file_desc->dondevoy));
			if (numero_a_leer + file_desc->dondevoy < 1024){
				leer_primero = numero_a_leer;
			}
		}
		int leidos = 0;
		int bloq_indirec = (bloq_ind[12 + 252*4 + 2]<<8)+bloq_ind[12 + 252*4+ 3];
		

		if (file_desc->bloque > 251){

			fseek(disk, 1024*bloq_indirec, SEEK_SET);
			fseek(disk,((file_desc->bloque) - 252)*4  , SEEK_CUR);
			unsigned char bloq[4];
			fread(bloq, 4,1,disk);
			int bloque_a_leer = (bloq[2]<<8)+bloq[3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(&buffer[leidos], leer_primero, 1, disk);


		}
		else {
			int bloque_a_leer = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(&buffer[leidos], leer_primero, 1, disk);
		}
		file_desc->dondevoy = 0;
		file_desc->bloque += 1;
		leidos += leer_primero;
		numero_a_leer -= leer_primero;
		while (numero_a_leer > 0){

			leer_primero = MIN(1024, numero_a_leer);

			if (file_desc->bloque > 251){

				fseek(disk, 1024*bloq_indirec, SEEK_SET);
				fseek(disk,((file_desc->bloque) - 252)*4  , SEEK_CUR);
				unsigned char bloq[4];
				fread(bloq, 4,1,disk);
				int bloque_a_leer = (bloq[2]<<8)+bloq[3];
				fseek(disk, 1024*bloque_a_leer, SEEK_SET);
				fseek(disk, (file_desc->dondevoy), SEEK_CUR);
				fread(&buffer[leidos], leer_primero, 1, disk);


			}
			else {
				int bloque_a_leer = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
				fseek(disk, 1024*bloque_a_leer, SEEK_SET);
				fseek(disk, (file_desc->dondevoy), SEEK_CUR);
				fread(&buffer[leidos], leer_primero, 1, disk);
			}
			if (leer_primero==1024){
				file_desc->dondevoy += 0;
				numero_a_leer -= leer_primero;
				leidos += leer_primero;
				file_desc->bloque+=1;
			}
			else {
				file_desc->dondevoy += leer_primero;
				numero_a_leer -= leer_primero;
				leidos += leer_primero;

			}
		}
		fclose(disk);
		clean_buffer(buffer, leidos);
		return leidos;
		

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
			clean_buffer(buffer, nbytes);
			return nbytes;


		}
		else {
			int bloque_a_leer = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_leer, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fread(buffer, nbytes, 1, disk);
			file_desc->dondevoy += nbytes;
			fclose(disk);
			clean_buffer(buffer, nbytes);
			return nbytes;
		}
	}

	fclose(disk);
	return -1;
}

int cz_write(czFILE* file_desc, void* buffer, int nbytes){

	if (!(file_desc)){
		fprintf(stderr,"cz_write: archivo inexistente, FILE* NULL \n");
		return -1;
	}
	FILE *disk;
	disk = fopen(discos,"r+b");
	if (!(file_desc->mode)){
		fprintf(stderr,"cz_write: %s abierto en modo 'r' \n", file_desc->nombre);
		return -1;
	}
	fseek(disk, 1024*(file_desc->indice), SEEK_SET);
	unsigned char bloq_ind[1024];
	fread(bloq_ind,1024,1,disk);
	int bloq_indirec = (bloq_ind[12 + 252*4 + 2]<<8)+bloq_ind[12 + 252*4+ 3];

	if ((file_desc->tamano)==0){
		
		//Buscar lugar para su bloque indice y inicializarlo
		fseek(disk, 1024, SEEK_SET);
		unsigned char bitmaps[8192];
		fread(bitmaps, sizeof(unsigned char), 8192, disk);
		fseek(disk, 1024, SEEK_SET);
		int bloque;
		int bit=0;
		unsigned char bits[8];
		int numero=0;
		int power;
		for (int n=0;n<8192;n++){
			if (bitmaps[n]!=255){
				bit = 0;
				for (int i = 0; i < 8; i++) {
				    bits[i] = (bitmaps[n] >> i) & 1;
				}
				for (int i = 7; i >=0; i--) {
					if (bits[i]==0){
						bit=i;
						bits[i] = 1;
						break;
					}
				}
				numero = 0;
				for (int i = 7; i >=0; i--) {
					power =  pow(2,i);
					numero += bits[i]*power;
				}
				bloque = 8*n + (7-bit);
				bitmaps[n] = numero;
				break;
			}
		}
		fseek(disk, 1024, SEEK_SET);
		fwrite(bitmaps, 8192, 1, disk);

		fseek(disk, 1024*bloque, SEEK_SET);
	    fwrite(buffer,nbytes,1, disk);

	    //escribir bloque de datos y retornar 
		fseek(disk, 12 + (file_desc->indice)*1024, SEEK_SET);
		unsigned char bloque_char[4];
		bloque_char[0] = (bloque >> 24) & 0xFF;
		bloque_char[1] = (bloque >> 16) & 0xFF;
		bloque_char[2] = (bloque >> 8) & 0xFF;
		bloque_char[3] = bloque & 0xFF;
		fwrite(bloque_char, 4,1,disk);

		file_desc->dondevoy += nbytes;
		file_desc->tamano += nbytes;

		//Escribir bloque indirecto
		fseek(disk, 1024, SEEK_SET);
		fread(bitmaps, sizeof(unsigned char), 8192, disk);
		fseek(disk, 1024, SEEK_SET);
		for (int n=0;n<8192;n++){
			if (bitmaps[n]!=255){
				bit=0;
				for (int i = 0; i < 8; i++) {
				    bits[i] = (bitmaps[n] >> i) & 1;
				}
				for (int i = 7; i >=0; i--) {
					if (bits[i]==0){
						bit=i;
						bits[i] = 1;
						break;
					}
				}
				numero=0;
				for (int i = 7; i >=0; i--) {
					power =  pow(2,i);
					numero += bits[i]*power;
				}
				bloque = 8*n + (7-bit);
				bitmaps[n] = numero;
				break;
			}
		}
		fseek(disk, 1024, SEEK_SET);
		fwrite(bitmaps, 8192, 1, disk);

	    //escribir bloque de datos y retornar 
		fseek(disk, 1020 + (file_desc->indice)*1024, SEEK_SET);
		bloque_char[0] = (bloque >> 24) & 0xFF;
		bloque_char[1] = (bloque >> 16) & 0xFF;
		bloque_char[2] = (bloque >> 8) & 0xFF;
		bloque_char[3] = bloque & 0xFF;
		fwrite(bloque_char, 4,1,disk);
		fclose(disk);
		
		return nbytes; 
		
	}
	else if ((file_desc->dondevoy) + nbytes > 1023){
		int numero_a_escribir = nbytes;
		int escritos=0;
		int a_escribir = 1024 - (file_desc->dondevoy);
		if (file_desc->bloque > 251){
			fseek(disk, 1024*bloq_indirec, SEEK_SET);
			fseek(disk, ((file_desc->bloque) - 252)*4  , SEEK_CUR);
			unsigned char bloq[4];
			fread(bloq, 4,1,disk);
			int bloque_a_escribir = (bloq[2]<<8)+bloq[3];
			fseek(disk, 1024*bloque_a_escribir, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fwrite(&buffer[escritos], a_escribir, 1, disk);
			escritos += a_escribir;
		}
		else {
			unsigned char algo[a_escribir];
			memcpy(algo,&buffer[escritos] , a_escribir);
			int bloque_a_escribir = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
			fseek(disk, 1024*bloque_a_escribir, SEEK_SET);
			fseek(disk, (file_desc->dondevoy), SEEK_CUR);
			fwrite(&buffer[escritos], a_escribir, 1, disk);
			escritos += a_escribir;
		}
		file_desc->dondevoy = 0;
		file_desc->tamano += a_escribir;
		file_desc->bloque += 1;
		numero_a_escribir -= a_escribir;
		while (numero_a_escribir > 0){

			//Buscar nuevo bloque
			fseek(disk, 1024, SEEK_SET);
			unsigned char bitmaps[8192];
			fread(bitmaps, sizeof(unsigned char), 8192, disk);
			fseek(disk, 1024, SEEK_SET);
			int bloque;
			int lleno = 1;
			for (int n=0;n<8192;n++){
				if (bitmaps[n]!=255){
					lleno = 0;
					unsigned char bits[8];
					int bit=0;
					for (int i = 0; i < 8; i++) {
					    bits[i] = (bitmaps[n] >> i) & 1;
					}
					for (int i = 7; i >=0; i--) {
						if (bits[i]==0){
							bit=i;
							bits[i] = 1;
							break;
						}
					}
					int numero=0;
					for (int i = 7; i >=0; i--) {
						int power =  pow(2,i);
						numero += bits[i]*power;
					}
					bloque = 8*n + (7-bit);
					bitmaps[n] = numero;
					break;
				}
			}
			if (lleno){
				fprintf(stderr, "%s\n", "Disco lleno");
				return escritos;
			}
			fseek(disk, 1024, SEEK_SET);
			fwrite(bitmaps, 8192, 1, disk);
			if (file_desc->bloque > 251){
				fseek(disk, bloq_indirec*1024+(252 - file_desc->bloque)*4, SEEK_SET);
				unsigned char bloque_char[4];
				bloque_char[0] = (bloque >> 24) & 0xFF;
				bloque_char[1] = (bloque >> 16) & 0xFF;
				bloque_char[2] = (bloque >> 8) & 0xFF;
				bloque_char[3] = bloque & 0xFF;
				fwrite(bloque_char, 4,1,disk);
			}
			else {
				fseek(disk, 12 + (file_desc->indice)*1024+(file_desc->bloque)*4, SEEK_SET);
				unsigned char bloque_char[4];
				bloque_char[0] = (bloque >> 24) & 0xFF;
				bloque_char[1] = (bloque >> 16) & 0xFF;
				bloque_char[2] = (bloque >> 8) & 0xFF;
				bloque_char[3] = bloque & 0xFF;
				fwrite(bloque_char, 4,1,disk);
			}
			fseek(disk, 1024*(file_desc->indice), SEEK_SET);
			fread(bloq_ind,1024,1,disk);
			bloq_indirec = (bloq_ind[12 + 252*4 + 2]<<8)+bloq_ind[12 + 252*4+ 3];

			//Escribir faltante en nuevo bloque
			int a_escribir = MIN(1024, numero_a_escribir);
			if (file_desc->bloque > 251){
				fseek(disk, 1024*bloq_indirec, SEEK_SET);
				fseek(disk, ((file_desc->bloque) - 252)*4  , SEEK_CUR);
				unsigned char bloq[4];
				fread(bloq, 4,1,disk);
				int bloque_a_escribir = (bloq[2]<<8)+bloq[3];
				fseek(disk, 1024*bloque_a_escribir, SEEK_SET);
				fseek(disk, (file_desc->dondevoy), SEEK_CUR);
				fwrite(&buffer[escritos], a_escribir, 1, disk);
				escritos += a_escribir;
			}
			else {
				int bloque_a_escribir = (bloq_ind[12 + (file_desc->bloque)*4 + 2]<<8)+bloq_ind[12 + (file_desc->bloque)*4+ 3];
				fseek(disk, 1024*bloque_a_escribir, SEEK_SET);
				fseek(disk, (file_desc->dondevoy), SEEK_CUR);
				fwrite(&buffer[escritos], a_escribir, 1, disk);
				escritos += a_escribir;
			}
			if (a_escribir==1024){
				file_desc->dondevoy = 0;
				file_desc->tamano += a_escribir;
				file_desc->bloque += 1;
				if ((file_desc->bloque)>507){
					fprintf(stderr, "Archivo %s %s\n", file_desc->nombre,"no puede crecer mas");
					return escritos;
				}
				numero_a_escribir -= a_escribir;
			}
			else {
				file_desc->dondevoy += a_escribir;
				file_desc->tamano += a_escribir;
				numero_a_escribir -= a_escribir;
			}

		}

		fclose(disk);
		return escritos;
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
	return -1;
}

int cz_close(czFILE* file_desc){
	if (file_desc==NULL){
		fprintf(stderr,"cz_close: archivo inexistente, FILE* NULL \n");
		return -1;
	}
	int nuevo_tam = file_desc->tamano;
	unsigned char tam[4];
	tam[0] = (nuevo_tam>>24) & 0xFF;
	tam[1] = (nuevo_tam>>16) & 0xFF;
	tam[2] = (nuevo_tam>>8) & 0xFF;
	tam[3] = nuevo_tam & 0xFF;
	
	if (file_desc->mode){
		FILE *disk;
		disk = fopen(discos,"r+b");
		fseek(disk, 1024*(file_desc->indice), SEEK_SET);
		fwrite(tam, sizeof(int),1, disk);
		fclose(disk); 

	}

	free(file_desc);

	return 0;
}

int cz_mv(char* orig, char *dest){
	FILE *disk;
	disk = fopen(discos,"r+b");
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
				fclose(disk); 
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
		fclose(disk); 
		return 1;
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
	return 0;
}

int cz_cp(char* orig, char* dest){
	printf("%s\n", "---- cp -----");
	FILE *disk;
	disk = fopen(discos,"r+b"); //escribir y leer en binario
	fclose(disk);
	return 1;
}

int cz_rm(char* filename){
	FILE *disk;
	disk = fopen(discos,"r+b"); //escribir y leer en binario

	int bloques_a_modificar[256+252];
	for (int i=0;i<508;i++){
		bloques_a_modificar[i]=0;
	}

	//Usar este bloque para reescribir los demas
	int bloque_en_cero[256];
	for (int i=0;i<256;i++){
		bloque_en_cero[i]=0;
	}



	int donde_en_primer = -1;  
	unsigned char valid[1];
	char name[11];
	unsigned char indice[4];
	fseek(disk, 0, SEEK_SET);
	unsigned char buffer[16];
	fread(buffer,16,1,disk);

	memcpy( valid, &buffer[0], 1);
	memcpy( name, &buffer[1], 11);
	memcpy( indice, &buffer[12], 4);
	int numero_bloque_indice;
	
	while ( (ftell(disk)<=1024)){
		if (*valid == 1){
			if (strcmp(name,filename) == 0){			
				donde_en_primer = ftell(disk) - 16;
				numero_bloque_indice = (indice[2]<<8)+indice[3];
				break;
			}
		}


		fread(buffer,16,1,disk);


		memcpy( valid, &buffer[0], 1);
		memcpy( name, &buffer[1], 11);
		memcpy( indice, &buffer[12], 4);
	}
	if (donde_en_primer==-1)
	{
		fprintf(stderr,"cz_rm: %s no existe \n", filename);
		fclose(disk); 
		return -1;
	}
	unsigned char bloque_indice[1024];
	unsigned char bloq_indirecto[1024];
	fseek(disk, 1024*numero_bloque_indice, SEEK_SET);
	fread(bloque_indice, 1024, 1, disk);
	int numero_bloque_indirecto = (bloque_indice[1022]<<8)+bloque_indice[1023];
	fseek(disk, 1024*numero_bloque_indirecto, SEEK_SET);
	fread(bloq_indirecto, 1024, 1, disk);

	unsigned char bloque_a_cambiar[4];
	int numero_bloque;
	int k=0;

	for (int j=0; j<1024; j+=4){
		memcpy(bloque_a_cambiar, &bloq_indirecto[j], 4);
		numero_bloque = (bloque_a_cambiar[2]<<8)+bloque_a_cambiar[3];
		if (numero_bloque !=0){
			fseek(disk, 1024*numero_bloque, SEEK_SET);
			fwrite(bloque_en_cero, 1024,1,disk);
			bloques_a_modificar[k] = numero_bloque;
			k++;
		}

	}
	for (int j=12; j<1020; j+=4){
		memcpy(bloque_a_cambiar, &bloque_indice[j], 4);
		numero_bloque = (bloque_a_cambiar[2]<<8)+bloque_a_cambiar[3];
		if (numero_bloque !=0){
			fseek(disk, 1024*numero_bloque, SEEK_SET);
			fwrite(bloque_en_cero, 1024,1,disk);
			bloques_a_modificar[k] = numero_bloque;
			k++;
		}

	}


	//liberar bloques en bitmap
	fseek(disk, 1024, SEEK_SET);
	unsigned char bitmaps[8192];
	fread(bitmaps, sizeof(unsigned char), 8192, disk);

	int n = numero_bloque_indirecto/8;
	float f = numero_bloque_indirecto/8.0 - numero_bloque_indirecto/8;
	unsigned char bits[8];
	for (int i = 0; i < 8; i++) {
	    bits[i] = (bitmaps[n] >> i) & 1;
	}
	//modificar bit correspondiente
	int a;
	for (int l=0; l<8;l++){
		if (l*0.125==f){
			a = l;
		}
	}
	bits[7-a] = 0;



	int numero=0;
	for (int i = 7; i >=0; i--) {
		int power =  pow(2,i);
		numero += bits[i]*power;
	}
	bitmaps[n] = numero;
	
	for (int j=0; j<508; j++){
		if (bloques_a_modificar[j]!=0){
			n = bloques_a_modificar[j]/8;
			f = bloques_a_modificar[j]/8.0 - bloques_a_modificar[j]/8;
			for (int i = 0; i < 8; i++) {
			    bits[i] = (bitmaps[n] >> i) & 1;
			}
			//modificar bit correspondiente
			for (int l=0; l<8;l++){
				if (l*0.125==f){
					a = l;
				}
			}
			bits[7-a] = 0;



			numero=0;
			for (int i = 7; i >=0; i--) {
				int power =  pow(2,i);
				numero += bits[i]*power;
			}
			bitmaps[n] = numero;
		}

	}
	n = numero_bloque_indice/8;
	f = numero_bloque_indice/8.0 - numero_bloque_indice/8;
	for (int i = 0; i < 8; i++) {
	    bits[i] = (bitmaps[n] >> i) & 1;
	}
	//modificar bit correspondiente
	for (int l=0; l<8;l++){
		if (l*0.125==f){
			a = l;
		}
	}
	
	bits[7-a] = 0;
	


	numero=0;
	for (int i = 7; i >=0; i--) {
		int power =  pow(2,i);
		numero += bits[i]*power;
	}
	bitmaps[n] = numero;

	fseek(disk, 1024, SEEK_SET);
	fwrite(bitmaps, 8192, 1, disk);



	//borrar bloque indice, indirecto y nombres
	fseek(disk, 1024*numero_bloque_indirecto, SEEK_SET);
	fwrite(bloque_en_cero, 1024, 1, disk);
	fseek(disk, 1024*numero_bloque_indice, SEEK_SET);
	fwrite(bloque_en_cero, 1024, 1, disk);

	int nombres[16];
	for (int i=0;i<16;i++){
		nombres[i]=0;
	}
	fseek(disk, donde_en_primer, SEEK_SET);
	fwrite(nombres, 16, 1, disk);



	fclose(disk);
	return 0;
}


void cz_ls(){
	FILE *disk;
	disk = fopen(discos,"rb");  // r for read, b for binary

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
			}
		
			printf("%s\n", name);

			int number = (indice[2]<<8)+indice[3];


		}
		fread(buffer,16,1,disk);


		memcpy( valid, &buffer[0], 1);
		memcpy( name, &buffer[1], 11);
		memcpy( indice, &buffer[12], 4);
	}

	fclose(disk);  


	
}
