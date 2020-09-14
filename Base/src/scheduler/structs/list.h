// Indica que este archivo solo se debe incluir una vez en la compilación
#pragma once
#include <time.h>

struct process;
typedef struct process Process;

// Definimos el struct nodo
struct process
{
  char name[31];
  int pid;
  int start_time;
  int deadline;
  int total_bursts;
  int cpu_bursts[255];
  int io_bursts[255];
  char status[10];

  /** Un puntero al siguiente nodo de la lista */
  Process* next;
};

// Declaramos el struct para la lista
struct linked_list;
// Definimos un alias para el struct lista
typedef struct linked_list List;

// Definimos el struct lista
struct linked_list
{
  /** El primer nodo de la lista */
  Process* head;
  /** El último nodo de la lista */
  Process* tail;
  int len;
};

// Declaramos las funciones asociadas

/** Inicializa una lista vacía */
List* list_init();
/** Inserta un nuevo elemento al final de la lista */
// void list_append(List* list, int pid);
/** Imprime todos los elementos de la lista */
void list_print(List* list);
/** Libera todos los recursos asociados a esta lista */
void list_destroy(List* list);
void list_remove(List* list, int pid);
void list_start_time_append(List* list, Process* process);
void list_deadline_append(List* list, Process* process);
Process* list_pop_head(List* list);
Process* list_pop_tail(List* list);
