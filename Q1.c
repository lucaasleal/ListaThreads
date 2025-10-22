#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; //criação estática.
int global_counter = 0;
 
void* counter(void* threadid)
{
  int* tid = ((int *) threadid);
  while (1)
  {
    pthread_mutex_lock(&mut); // Isso garante que APENAS UMA thread possa executar o código
    global_counter++;
    if (global_counter >= 1000000) //Verifica se já terminou a contagem após incrementar na thread atual
    {
      printf("thread %d ganhou a corrida com valor %d\n", *tid, global_counter);
      exit(0);
    }
    pthread_mutex_unlock(&mut); // Destrava o mutex para que outras threads possam acessar o contador
  }

  pthread_exit(NULL);
}

int main()
{

  //Inicialização das Threads
  pthread_t threads[NUM_THREADS];
  int threadid[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++)
    threadid[i] = i;

  for (int i = 0; i < NUM_THREADS; i++)
  {
    int rc = pthread_create(&threads[i], NULL, counter, &threadid[i]); // A thread é criada e recebe seu ID
    if (rc) {printf("Erro detectado\n"); return 1;}
  }

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);
  pthread_join(threads[3], NULL);

  //Destruição do mutex
  pthread_mutex_destroy(&mut);

  pthread_exit(NULL);
}