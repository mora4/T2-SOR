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
    process->current_cpu_burst = 0;
    process->current_io_burst = 0;
    process->burst_time_left = -1;
    process -> new_waiting = 0;
    process -> next = NULL;
    process -> running_times = 0;
    process -> interrupted_times = 0;
    process -> turnaround_time;
    process -> response_time = -1;
    process -> waiting_time = 0;
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
      // Le asignamos el cpu burst que le toca
      ready_process ->burst_time_left = ready_process->cpu_bursts[ready_process->current_cpu_burst];
      list_deadline_append(queue->ready_processes, ready_process);
    }
}

void running_processes(Queue* queue, int cpus, int t) 
{
  int finished_list[255];
  int finished = 0;
  int waiting_list[255];
  int waiting = 0;
  for(Process* current = queue->running_processes -> head; current; current = current -> next)
  {
    if (current->response_time == -1)
    {
      current->response_time = t - current->start_time;
    }

    current->burst_time_left -= 1;
    
    if (current->burst_time_left == 0)
    {
      current -> current_cpu_burst += 1;
      if (current -> current_cpu_burst == current -> total_bursts)
      {
        finished_list[finished] = current ->pid;
        finished++;
      }
      else 
      {
        waiting_list[waiting] = current ->pid;
        waiting++;
        current->burst_time_left = current->io_bursts[current->current_io_burst];
      }
    }
  }

  for (int i = 0; i < finished; i++)
  {
    Process* finished_process = list_remove(queue ->running_processes, finished_list[i]);
    list_deadline_append(queue->finished_processes, finished_process);
    finished_process->turnaround_time = t + 1 - finished_process->start_time;
    finished_process->in_time = t <= finished_process->deadline;
  }

  for (int i = 0; i < waiting; i++)
  {
    Process* waiting_process = list_remove(queue ->running_processes, waiting_list[i]);
    waiting_process -> new_waiting = 1;
    list_deadline_append(queue->waiting_processes, waiting_process);
  }
}

void ready_processes(Queue* queue, int cpus) 
{
  // Si no hay ready processes no hacemos nada
  if (queue->ready_processes->head == NULL) return;

  // Cantidad de procesos que podemos pasar a running
  int ready = cpus - queue->running_processes->len;
  for (int i = 0; i < ready; i++)
  {
    // Pasamos a todos los que caben en running
    if (queue ->ready_processes->head)
    {
      Process* ready_process = list_pop_head(queue ->ready_processes);
      list_deadline_append(queue->running_processes, ready_process);
      ready_process->running_times += 1;
    }
    else 
    {
      // Ya no quedan procesos en la cola ready
      break;
    }
  }

  // Si no quedan ready processes no hay nada que interrumpir
  if (queue->ready_processes->head == NULL) return;

  // Vemos si hay que interrumpir un proceso
  int check_for_interrupt = 1;
  while(check_for_interrupt)
  {
    //Si el proceso tiene menor deadline que el último de la cola running o es igual y tiene menor ID
    if ((queue->ready_processes->head) && 
       ((queue->ready_processes->head->deadline < queue->running_processes->tail->deadline) || 
       ((queue->ready_processes->head->deadline == queue->running_processes->tail->deadline) &&
       (queue->ready_processes->head->pid < queue->running_processes->tail->pid))))
    {
      Process* ready_process = list_pop_head(queue ->ready_processes);
      Process* interrupted_process = list_pop_tail(queue ->running_processes);
      list_deadline_append(queue->running_processes, ready_process);
      ready_process->running_times += 1;
      list_deadline_append(queue->ready_processes, interrupted_process);
      interrupted_process->interrupted_times += 1;
    }
    else
    {
      check_for_interrupt = 0;
    } 
  }
  
  for(Process* current = queue->ready_processes -> head; current; current = current -> next)
  {
    current->waiting_time +=1;
  }
}

void waiting_processes(Queue* queue) 
{
  int ready_list[255];
  int ready = 0;

  for(Process* current = queue->waiting_processes -> head; current; current = current -> next)
  {
    if (current->burst_time_left == 0)
    {
      current -> current_io_burst += 1;
      ready_list[ready] = current ->pid;
      ready++;
      current->burst_time_left = current->cpu_bursts[current->current_cpu_burst];
    }
    else
    {
      current->burst_time_left -= 1;
      current->waiting_time += 1;
    }
  }

  for (int i = 0; i < ready; i++)
  {
    Process* ready_process = list_remove(queue ->waiting_processes, ready_list[i]);
    list_deadline_append(queue->ready_processes, ready_process);
  }
}

void stats (Queue* queue, char* out_filename)
{
  FILE *file = fopen(out_filename, "w");
  for(Process* current = queue->finished_processes -> head; current; current = current -> next)
  {
    fprintf(file, "%s,%i,%i,%i,%i,%i,%i\n", current->name, current->running_times, current->interrupted_times, current->turnaround_time, current->response_time, current->waiting_time, current->in_time);
  }
  fclose(file);
}

void simulation(Queue* queue, int cpus)
{
  int t = -1;
  while (queue->not_started_processes->len>0 || queue->ready_processes->len>0 || queue->waiting_processes->len>0 
  || queue->running_processes->len>0)
  {
    t+=1;
    new_processes(queue, cpus, t);
    waiting_processes(queue);
    ready_processes(queue, cpus);
    running_processes(queue, cpus, t);
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
    stats(queue, argv[2]);
    queue_destroy(queue);
  }
  else if (argc == 4)
  {
    int cpus = atoi(argv[3]);
    Queue* queue = read_input(argv[1]);
    simulation(queue, cpus);
    stats(queue, argv[2]);
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
