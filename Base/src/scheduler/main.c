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

void read_input(char* filename)
{
  // Abrimos el archivo
  FILE* file = fopen(filename, "r");

  // Leemos la cantidad de procesos
  int total_processes;
  fscanf(file, "%i", &total_processes);
  printf("Numero de procesos: %i\n", total_processes);

  Process processes[total_processes];

  char line[256];

  // Leemos la primera fila (que ya habiamos leido)
  fgets(line, sizeof(line), file);
  // Inicializar cada proceso
  for (int p = 0; p < total_processes; p++)
  {

    fgets(line, sizeof(line), file);
    printf("Proceso numero %i: %s\n", p, line);

    const char s[2] = " ";
    char *token;

    /* get the process name */
    token = strtok(line, s);
    strcpy(processes[p].name, token);
    printf( "Nombre: %s\n", processes[p].name);

    /* get the pid */
    token = strtok(NULL, s);
    char pid_str[255];
    strcpy(pid_str, token);
    int pid = atoi(pid_str);
    processes[p].pid = pid;
    printf( "PID: %i\n", processes[p].pid);

    /* get the start time */
    token = strtok(NULL, s);
    char start_time_str[255];
    strcpy(start_time_str, token);
    int start_time = atoi(start_time_str);
    processes[p].start_time = start_time;
    printf( "Start time: %i\n", processes[p].start_time);

    /* get the deadline */
    token = strtok(NULL, s);
    char deadline_str[255];
    strcpy(deadline_str, token);
    int deadline = atoi(deadline_str);
    processes[p].deadline = deadline;
    printf( "Deadline: %i\n", processes[p].deadline);

    /* get the bursts */
    token = strtok(NULL, s);
    char bursts_str[255];
    strcpy(bursts_str, token);
    int bursts = atoi(bursts_str);
    processes[p].total_bursts = bursts;
    printf( "Total bursts: %i\n", processes[p].total_bursts);

    for (int b = 0; b < (processes[p].total_bursts * 2) - 1; b++)
    {
      token = strtok(NULL, s);
      char burst_str[255];
      strcpy(burst_str, token);
      int burst = atoi(burst_str);
      printf("b: %i, burst: %i\n", b, burst);

      if(b % 2 == 0) {
        processes[p].cpu_bursts[b/2] = burst;
      } 
      else {
        processes[p].io_bursts[b/2] = burst; 
      }
    }
    
    for (int b = 0; b < processes[p].total_bursts - 1; b++)
    {
      printf("CPU burst %i: %i\n", b, processes[p].cpu_bursts[b]);
      printf("IO burst %i: %i\n", b, processes[p].io_bursts[b]);
    }
    
  }
  fclose(file);
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
    read_input(argv[1]);
  }
  else if (argc == 4)
  {
    int cpus = atoi(argv[3]);
    printf("CPUs: %i", cpus);
    read_input(argv[1]); 
    
  }
  else
  {
    printf("Cantidad de parámetros incorrecta\n");
    printf("Uso correcto: %s PATH_TO_INPUT\n", argv[0]);
    return 1;
  }

  

  return 0;
}
