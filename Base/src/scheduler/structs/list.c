#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>


/** Inicializa una lista vacía */
List* list_init()
{
  List* list = malloc(sizeof(List));
  list -> head = NULL;
  list -> tail = NULL;

  return list;
}

/** Inserta un nuevo elemento al final de la lista */
// void list_append(List* list, Args* args, int pid)
// {
  // Process* process = malloc(sizeof(Process));
//   process -> pid = pid;
//   memcpy (process->command, args->argv[0], sizeof(args->command)); 
//   process -> start_time = time(NULL);
//   process -> next = NULL;

//   // Si la lista está vacía entonces queda como el primer elemento
//   if(list -> head == NULL)
//   {
//     list -> head = process;
//   }
//   // Sino, se pone como el siguiente del ultimo
//   else
//   {
//     list -> tail -> next = process;
//   }
//   // Luego se deja a este nodo como el último
//   list -> tail = process;
// }

/** Imprime todos los elementos de la lista */
void list_print(List* list)
{
  for(Process* current = list -> head; current; current = current -> next)
  {
    time_t current_time = time(NULL);
    printf("PID: %i      Tiempo: %li\n", current->pid, current_time - current->start_time);
  }
}

/** Libera todos los recursos asociados a esta lista */
void list_destroy(List* list)
{
  // Liberamos los nodos
  if(list -> head)
  {
    Process* curr = list -> head -> next;
    Process* prev = list -> head;

    while(curr)
    {
      free(prev);
      prev = curr;
      curr = curr -> next;
    }
    free(prev);
  }
  // Liberamos la lista en si
  free(list);
}

/** Remueve al proceso con pid pid */
void list_remove(List* list, int pid)
{
    Process* curr = list -> head -> next;
    Process* prev = list -> head;

    if(prev->pid == pid)
    {
        if(list->tail->pid == pid)
        {
            list->tail = NULL;
        }
        list->head = list->head->next;
        // printf("Voy a destruir al nodo HEAD con pid: %i = %i", prev->pid, pid);
        free(prev);
    }
    else
    {
        while((curr->pid != pid) && curr)
        {
            prev = curr;
            curr = curr -> next;
        }
        // printf("Voy a destruir al nodo con pid: %i = %i", curr->pid, pid);
        prev ->next = curr->next;

        free(curr);    
    }
}

/** Remueve los procesos terminados */
void list_remove_exited(List* list)
{
  if (list->head)
  {
    Process* curr = list -> head -> next;
    Process* prev = list -> head;
    pid_t pid_return;
    int status;

    // Reviso hasta tener un head que no haya terminado
    while(list->head && list->head->pid != 0)
    {
      pid_return = waitpid(list->head->pid, &status, WNOHANG);
      // printf("pid_return: %i\n", pid_return);
      if ( pid_return && (WIFEXITED(status) || WIFSIGNALED(status))) 
      { 
        prev = list -> head;
        list->head = list->head->next;
        // printf("Eliminando PID: %i\n", prev->pid);
        free(prev);
      }
      else
      {
        break;
      }
    }
    
    prev = list->head;
    
    if (prev)
    {
      curr = prev ->next;
      while(curr)
      {
        waitpid(curr->pid, &status, WNOHANG);
        // printf("pid_return: %i\n", pid_return);
        if (pid_return && (WIFEXITED(status) || WIFSIGNALED(status))) 
        { 
          if (list->tail->pid == curr->pid)
          {
              list->tail = prev;
          }
          prev ->next = curr ->next;
          // printf("Eliminando PID: %i\n", curr->pid);
          free(curr);
          curr = prev->next;
        }
        else
        {
          prev = curr;
          curr = curr -> next;    
        }    
      }
    }
  }
}

/** Envia señal a todos los procesos */
void list_send_signal(List* list, int signal)
{
  for(Process* current = list -> head; current; current = current -> next)
  {
    int kill_return = kill(current->pid, signal);
    if (kill_return != 0)
    {
      printf("KILL FAILED: %s\n", strerror(errno));
    }
  }
}