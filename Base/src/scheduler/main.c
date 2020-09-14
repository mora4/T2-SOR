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
      printf("b: %i, burst: %i\n", b, burst);

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

int main(int argc, char *argv[])
{
  // Este programa recibe dos parámetros:
  //  argv[0] = el programa en sí
  //  argv[1] = la ruta al archivo de input
  if (argc == 3)
  {
    int cpus = 1;
    printf("CPUs: %i", cpus);
    Queue* queue =read_input(argv[1]);
  }
  else if (argc == 4)
  {
    int cpus = atoi(argv[3]);
    printf("CPUs: %i", cpus);
    Queue* queue = read_input(argv[1]); 
    
  }
  else
  {
    printf("Cantidad de parámetros incorrecta\n");
    printf("Uso correcto: %s PATH_TO_INPUT\n", argv[0]);
    return 1;
  }

  return 0;
}
