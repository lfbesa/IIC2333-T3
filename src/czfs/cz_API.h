#pragma once

typedef struct cz_file {
	char *  _ptr;
	int mode; // 1 es w y 0 es r
	int indice;
	int dondevoy;
	int bloque;
	char nombre[11];
	int tamano;

} czFILE;

void cz_mount_disco(char *virtua);

void clean_buffer(char *buffer, int donde);

czFILE* cz_open(char *disco, char* filename, char mode);

int cz_exists(char* filename);

int cz_read(char *disco, czFILE* file_desc, void* buffer, int nbytes);

int cz_write(char *disco, czFILE* file_desc, void* buffer, int nbytes);

int cz_close(char *disco, czFILE* file_desc);

int cz_mv(char* orig, char *dest);

int cz_cp(char *disco, char* orig, char* dest);

int cz_rm(char *disco, char* filename);

void cz_ls();