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
  list->len = 0;

  return list;
}

/** Inserta un nuevo elemento segun su start time */
void list_start_time_append(List* list, Process* process)
{
  list->len++;
    process -> next = NULL;

  // Si la lista está vacía entonces queda como el primer elemento
  if(list -> head == NULL)
  {
    list -> head = process;
    list ->tail = process;
  }
  // Caso en que es mas prioritario que el head actual
  else if (list -> head -> start_time > process -> start_time) 
  {
    process -> next = list -> head;
    list -> head = process;
  }
  // Sino, se pone antes del que tenga un start time mayor
  else
  {
    Process* curr = list -> head -> next;
    Process* prev = list -> head;
    while(curr)
    {
      if (curr -> start_time > process -> start_time)
      {
        prev -> next = process;
        process -> next = curr;
        return;
      }

      prev = curr;
      curr = curr -> next;
    }
      prev -> next = process;
      list -> tail = process;
      return;
  }
}

/** Inserta un nuevo elemento segun su start time */
void list_deadline_append(List* list, Process* process)
{
  process -> next = NULL;
  list->len++;

  // Si la lista está vacía entonces queda como el primer elemento
  if(list -> head == NULL)
  {
    list -> head = process;
    list ->tail = process;
  }
  // Caso en que es mas prioritario que el head actual
  else if (list -> head -> deadline > process -> deadline) 
  {
    process -> next = list -> head;
    list -> head = process;
  }
  // Sino, se pone antes del que tenga un start time mayor
  else
  {
    Process* curr = list -> head -> next;
    Process* prev = list -> head;
    while(curr)
    {
      if (curr -> deadline > process -> deadline)
      {
        prev -> next = process;
        process -> next = curr;
        return;
      }

      prev = curr;
      curr = curr -> next;
    }
    prev -> next = process;
    list -> tail = process;
    return;
  }
}

/** Saca al primer elemento de la lista */
Process* list_pop_head(List* list)
{
  Process* head = list->head;
  list -> head = list->head->next;
  list->len--;
  return head;
}

/** Saca al ultimo elemento de la lista */
Process* list_pop_tail(List* list)
{
  Process* curr = list -> head -> next;
  Process* prev = list -> head;

  if (curr == NULL)
  {
    list->head = NULL;
    list->tail = NULL;
    return prev;
  }
  else
  {
    while(curr->next)
    {
      prev = curr;
      curr = curr -> next;
    }
    list->tail = prev;
    list ->tail->next = NULL;
    return curr;
  }
}

/** Imprime todos los elementos de la lista */
void list_print(List* list)
{
  for(Process* current = list -> head; current; current = current -> next)
  {
    printf("PID: %i      Start time: %i   Deadline: %i\n", current->pid, current->start_time, current->deadline);
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