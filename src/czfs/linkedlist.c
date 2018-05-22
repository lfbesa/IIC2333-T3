// importo el archivo linkedlist.h
#include "linkedlist.h"
// Libreria estandar de C
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
//                             Funciones                                //
//////////////////////////////////////////////////////////////////////////

// TODO: debes completar las funciones que estan a continuacion
// Puedes crear otras funciones aca para el
// funcionamiento interno del arreglo dinamico

/** Crea un nodo */
Node* node_init(Process* value)
{
  // Creo el nodo
  Node* node = malloc(sizeof(Node));

  // Le agrego el valor y le pongo next = NULL
  node -> value = value;
  node -> next = NULL;

  // Retorno el nodo
  return node;
}

/** Libera los nodos recursivamente */
void recursive_destroy(Node* node)
{
  // Si tiene sucesor, llama recursivamente
  if (node -> next)
  {
    recursive_destroy(node -> next);
  }

  // Libero al nodo
  free(node->value);
  free(node);
}

/** Crea una lista inicialmente vacia y retorna el puntero */
LinkedList* linkedlist_init()
{
  // Creo la linked list
  LinkedList* list = malloc(sizeof(LinkedList));

  // Relleno sus atributos
  list -> count = 0;
  list -> first = NULL;
  list -> last = NULL;
  list -> next = NULL;

  // Retorno la lista
  return list;
}

/** Inserta un elemento al final de la lista */
void linkedlist_append(LinkedList* list, Process* element)
{
  // Creo el nodo a agregar
  Node* node = node_init(element);

  // Si la lista esta vacia
  if (list -> count == 0)
  {
    // Hago que sea el primer nodo
    list -> first = node;
  }
  // Sino,
  else
  {
    // Hago que sea el siguiente del ultimo
    list -> last -> next = node;
  }

  // Ahora este nodo es el ultimo
  list -> last = node;

  // Agrego 1 a la cuenta
  list -> count++;
}

/** Inserta el elemento dado en la posicion indicada */
void linkedlist_insert(LinkedList* list, Process* element, int position)
{
  // Creo el nodo a insertar
  Node* node = node_init(element);

  // Si me piden insertar en la posicion 0:
  if (position == 0)
  {
    // Hago que referencie al primero
    node -> next = list -> first;
    // Hago el nodo nuevo como el primero
    list -> first = node;
  }
  // Si insertan en la ultima posicion
  else if (position == list -> count)
  {
    // Hago que sea el siguiente del ultimo
    list -> last -> next = node;

    // Ahora este nodo es el ultimo
    list -> last = node;
  }
  // Si inserto entremedio
  else
  {
    // Itero sobre la lista hasta encontrar la posicion
    Node* actual = list -> first;
    for (int i = 1; i < position; i++)
    {
      actual = actual -> next;
    }
    // Hago que el nodo apunte al siguiente
    node -> next = actual -> next;
    // Hago que el anterior apunte al nodo
    actual -> next = node;
  }

  // Actualizo la cantidad
  list -> count++;
}

/** Elimina el elemento de la posicion indicada y lo retorna */
Process* linkedlist_delete(LinkedList* list, int position)
{
  // Variable a retornar
  Process* value;

  // Si me piden eliminar el primero
  if (position == 0)
  {
    // Obtengo el valor a retornar
    value = list -> first -> value;

    // Guardo el primero
    Node* first = list -> first;

    // Hago que el primero sea ahora el siguiente
    list -> first = first -> next;

    // Elimino el anterior
    free(first);
  }
  // En cuelquier otro caso
  else
  {
    // Busco el nodo anterior al que voy a eliminar
    Node* last = list -> first;
    for (int i = 1; i < position; i++)
    {
      last = last -> next;
    }

    // Obtengo el nodo a eliminar
    Node* actual = last -> next;

    // Obtengo el valor a retornar
    value = actual -> value;

    // Cambio el link del anterior al siguiente
    last -> next = actual -> next;

    // Libero la memoria del nodo eliminado
    free(actual);

    // Si elimine el ultimo, actualizo cual es la posicion final de la lista
    if (position == list -> count - 1)
    {
      list -> last = last;
    }
  }

  // Disminuyo en 1 el count
  list -> count--;

  // retorno el valor eliminado
  return value;
}

/** Retorna el valor del elemento en la posicion dada */
Process* linkedlist_get(LinkedList* list, int position)
{
  // Itero sobre la lista buscando el valor a retornar
  Node* actual = list -> first;
  for (int i = 0; i < position; i++)
  {
    actual = actual -> next;
  }

  // Retorno el valor que hay en el nodo
  return actual -> value;
}

/** Libera todos los recursos asociados a la lista */
void linkedlist_destroy(LinkedList* list)
{
  // Libero todos los nodos
  if (list->count>0){
     recursive_destroy(list -> first);
  }
  // Libero la lista
  free(list);
}



SuperLinkedList* super_linkedlist_init()
{
  // Creo la linked list
  SuperLinkedList* list = malloc(sizeof(SuperLinkedList));

  // Relleno sus atributos
  list -> count = 0;
  list -> first = NULL;
  list -> last = NULL;

  // Retorno la lista
  return list;
}
void superlinkedlist_append(SuperLinkedList* list, int priori)
{
  // Creo el nodo a agregar
  LinkedList* lista = linkedlist_init();
  lista->prioridad = priori;

  // Si la lista esta vacia
  if (list -> count == 0)
  {
    // Hago que sea el primer nodo
    list -> first = lista;
  }
  // Sino,
  else
  {
    // Hago que sea el siguiente del ultimo
    list -> last -> next = lista;
  }

  // Ahora este nodo es el ultimo
  list -> last = lista;

  // Agrego 1 a la cuenta
  list -> count++;
}
LinkedList* superlinkedlist_get(SuperLinkedList* list, int position)
{
  // Itero sobre la lista buscando el valor a retornar
  LinkedList* actual = list -> first;
  for (int i = 0; i < position; i++)
  {
    actual = actual -> next;
  }

  // Retorno el valor que hay en el nodo
  return actual;
}
void super_linkedlist_destroy(SuperLinkedList* list)
{
  // Libero todos los nodos
  for (int n=0; n<list->count; n++){
    linkedlist_destroy(superlinkedlist_get(list, n));
  }
  // Libero la lista
  free(list);
}
