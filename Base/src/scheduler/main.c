#include "./structs/queue.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

Queue* read_input(char* filename)
{
  // Abrimos el archivo
  FILE* file = fopen(filename, "r");

  // Leemos la cantidad de procesos
  int total_processes;
  fscanf(file, "%i", &total_processes);

  Queue* queue = queue_init();


  char line[256];

  // Leemos la primera fila (que ya habiamos leido)
  fgets(line, sizeof(line), file);
  // Inicializar cada proceso
  for (int p = 0; p < total_processes; p++)
  {
    Process* process = malloc(sizeof(Process));
    process -> next = NULL;
    fgets(line, sizeof(line), file);

    const char s[2] = " ";
    char *token;

    /* get the process name */
    token = strtok(line, s);
    strcpy(process->name, token);

    /* get the pid */
    token = strtok(NULL, s);
    char pid_str[255];
    strcpy(pid_str, token);
    int pid = atoi(pid_str);
    process->pid = pid;


    /* get the start time */
    token = strtok(NULL, s);
    char start_time_str[255];
    strcpy(start_time_str, token);
    int start_time = atoi(start_time_str);
    process->start_time = start_time;
 

    /* get the deadline */
    token = strtok(NULL, s);
    char deadline_str[255];
    strcpy(deadline_str, token);
    int deadline = atoi(deadline_str);
    deadline = deadline + process->start_time;
    process->deadline= deadline;

    /* get the bursts */
    token = strtok(NULL, s);
    char bursts_str[255];
    strcpy(bursts_str, token);
    int bursts = atoi(bursts_str);
    process->total_bursts = bursts;

    for (int b = 0; b < (bursts * 2) - 1; b++)
    {
      token = strtok(NULL, s);
      char burst_str[255];
      strcpy(burst_str, token);
      int burst = atoi(burst_str);

      if(b % 2 == 0) {
        process->cpu_bursts[b/2] = burst;
      } 
      else {
        process->io_bursts[b/2] = burst; 
      }
    }
    
    list_start_time_append(queue->not_started_processes, process);
  }
  fclose(file);
  return queue;
}

void new_processes(Queue* queue, int cpus, int t) 
{
  while (queue->not_started_processes->head && queue->not_started_processes->head->start_time == t)
    //Proceso entra a la simulación
    {
      Process* ready_process = list_pop_head(queue->not_started_processes);
      //Si el proceso tiene menor deadline que el último de la cola running o es igual y tiene menor ID
      if(queue->running_processes->len == cpus && (
        (ready_process->deadline<queue->running_processes->tail->deadline) ||
        ((ready_process->deadline==queue->running_processes->tail->deadline) 
        && (ready_process->pid<queue->running_processes->tail->pid))))
      {
        Process* interrupted_process = list_pop_tail(queue->running_processes);
        list_deadline_append(queue->running_processes, ready_process);
        list_deadline_append(queue->ready_processes, interrupted_process);    
      }
      list_deadline_append(queue->ready_processes, ready_process);
    }
}

void simulation(Queue* queue, int cpus)
{
  int t = -1;
  while ((t<5) && (queue->not_started_processes->len>0 || queue->ready_processes->len>0 || queue->waiting_processes->len>0 
  || queue->running_processes->len>0))
  {
    t+=1;
    new_processes(queue, cpus, t);
    printf("Time: %i\n", t);
    printf("Ready:\n");
    list_print(queue->ready_processes);
    printf("Not started:\n");
    list_print(queue->not_started_processes);
  }
  
}

int main(int argc, char *argv[])
{
  // Este programa recibe dos parámetros:
  //  argv[0] = el programa en sí
  //  argv[1] = la ruta al archivo de input
  if (argc == 3)
  {
    int cpus = 1;
    Queue* queue =read_input(argv[1]);
    simulation(queue, cpus);
    queue_destroy(queue);
  }
  else if (argc == 4)
  {
    int cpus = atoi(argv[3]);
    Queue* queue = read_input(argv[1]);
    simulation(queue, cpus);
    queue_destroy(queue);
  }
  else
  {
    printf("Cantidad de parámetros incorrecta\n");
    printf("Uso correcto: %s PATH_TO_INPUT\n", argv[0]);
    return 1;
  }
  

  return 0;
}
