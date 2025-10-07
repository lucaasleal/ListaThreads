#define BUFFER_SIZE 5
#define PROGRAM_EXECUTION_LIMIT 500

#define C 2
#define P 1

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

//1: DECLARAÇÕES

pthread_mutex_t MUTEX_BUFFER_ACC = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  PC_FULL = PTHREAD_COND_INITIALIZER;
pthread_cond_t  PC_EMPTY= PTHREAD_COND_INITIALIZER;

int buffer[BUFFER_SIZE];
int item = 0; //LIFO
long int counter = 0;

//2. THREAD CONSUMIDOR

void* CONS_ROUTINE(void* TREADPARAMS)
{
  while (1)
  {
    int* value= ((int *) TREADPARAMS); 

    pthread_mutex_lock(&MUTEX_BUFFER_ACC);
    if (counter >= PROGRAM_EXECUTION_LIMIT) exit(0);
    if (item < 0) {printf("waiting for producer...\n"); pthread_cond_wait(&PC_EMPTY, &MUTEX_BUFFER_ACC);}
    while (item < 0) pthread_cond_wait(&PC_EMPTY, &MUTEX_BUFFER_ACC);

    if (item >= BUFFER_SIZE) item = BUFFER_SIZE - 1;
    int READ_VAL = buffer[item--];
    printf("thread %d read value %d from buffer position %d\n", *value, READ_VAL, item + 1);
    if (item < BUFFER_SIZE) pthread_cond_signal(&PC_FULL);

    pthread_mutex_unlock(&MUTEX_BUFFER_ACC);
  }

  pthread_exit(NULL);
}

//3. THREAD PRODUTOR:

void* PROD_ROUTINE(void* THREADPARAMS)
{
  while (1)
  {
    pthread_mutex_lock(&MUTEX_BUFFER_ACC);
    if (counter >= 500) exit(0);
    if (item >= BUFFER_SIZE) {printf("waiting for consumer...\n"); pthread_cond_wait(&PC_FULL, &MUTEX_BUFFER_ACC);}
    while (item >= BUFFER_SIZE) pthread_cond_wait(&PC_FULL, &MUTEX_BUFFER_ACC);


    if (item <= 0) item = 0;
    buffer[item++] = counter++;
    printf("wrote value %ld to buffer position %d\n", counter - 1, item - 1);
    if (item >= 0) pthread_cond_signal(&PC_EMPTY);
    pthread_mutex_unlock(&MUTEX_BUFFER_ACC);
  }

  pthread_exit(NULL);
} 

//4. THREAD MAIN:

int main()
{
  
  int taskidCONS[C];
  int taskidPROD[P];
  
  pthread_t CONS[C];
  pthread_t PROD[P];
  

  /*
  pthread_t PROD;
  pthread_t CONS1, CONS2, CONS3;

  int taskid[] = {0,1,2,3};
  
  int rcPROD = pthread_create(&PROD, NULL, PROD_ROUTINE,   (void*)&taskid[0]);
  int rcCONS1 = pthread_create(&CONS1, NULL, CONS_ROUTINE, (void*)&taskid[1]);
  int rcCONS2 = pthread_create(&CONS2, NULL, CONS_ROUTINE, (void*)&taskid[2]);
  int rcCONS3 = pthread_create(&CONS3, NULL, CONS_ROUTINE, (void*)&taskid[3]);
  

  if ((rcPROD || rcCONS1 || rcCONS2 || rcCONS3) == 1)
  {
    printf("erro na criação das threads\n"); exit(-1);
  }
  
  */

  
  for (int i = 0; i < C; i++)
  {
    taskidCONS[i] = i;
    int rcCONS = pthread_create(&CONS[i], NULL, CONS_ROUTINE, (void*)&taskidCONS[i]);
    if (rcCONS) 
    {
      printf("teve erro ai chefe\n");
      exit(1);
    }
  }
  
  for(int i = 0; i < P; i++)
  {
    int rcPROD = pthread_create(&PROD[i], NULL, PROD_ROUTINE, (void*)&taskidPROD[i]);
    if (rcPROD) 
    {
      printf("teve erro ai chefe\n");
      exit(1);
    }
  }
  
  pthread_exit(NULL);
}