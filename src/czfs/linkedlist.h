// Esta linea sirve para que el codigo de este archivo solo se importe una vez
#pragma once

struct process
{
	char nombre[256];
	int pid;
	int estado;
	int prioridad;
	int burst[40];
	int n_burst;
  int turnaround;
  int inicio;
  int nro_procesos;
  int nro_int;
  int waiting;
  int response;
  int atendido;
};
typedef struct process Process;

/** Estructura de un nodo de lista */
struct node
{
  /** Siguiente nodo */
  struct node* next;
  /** Valor del nodo */
  Process* value;
};

typedef struct node Node;


/** Estructura de una lista ligada */
struct linked_list
{
  /** Primer nodo */
  Node* first;
  /** Ultimo nodo */
  Node* last;
  /** Cantidad de nodos */
  int count;
  struct linked_list* next;
  int prioridad;
};

// Aqui le estoy poniendo un nombre mas simple a la lista para no tener que
// referirme a la lista como struct linked_list
/** Estructura de una lista ligada */
typedef struct linked_list LinkedList;
struct Queue
{
  /** Primer nodo */
  LinkedList* first;
  /** Ultimo nodo */
  LinkedList* last;
  /** Cantidad de nodos */
  int count;
};
typedef struct Queue SuperLinkedList;


//////////////////////////////////////////////////////////////////////////
//                             Funciones                                //
//////////////////////////////////////////////////////////////////////////

//OJO: No se debe modificar nada de esto

/** Crea una lista inicialmente vacia y retorna el puntero */
LinkedList* linkedlist_init();

/** Inserta un elemento al final de la lista */
void linkedlist_append(LinkedList* list, Process* element);

/** Inserta el elemento dado en la posicion indicada */
void linkedlist_insert(LinkedList* list, Process* element, int position);

/** Elimina el elemento de la posicion indicada y lo retorna */
Process* linkedlist_delete(LinkedList* list, int position);

/** Retorna el valor del elemento en la posicion dada */
Process* linkedlist_get(LinkedList* list, int position);

/** Libera todos los recursos asociados a la lista */
void linkedlist_destroy(LinkedList* list);

SuperLinkedList* super_linkedlist_init();

void superlinkedlist_append(SuperLinkedList* list, int prioridad);

LinkedList* superlinkedlist_get(SuperLinkedList* list, int position);

void super_linkedlist_destroy(SuperLinkedList* list);