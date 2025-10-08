#include <stdio.h>
#include <pthread.h> 
#include <stdlib.h>

#define BUFFER_SIZE 10
#define NUM_ITEMS 5000

#define C 50
#define P 50
#define B 1

int buff[BUFFER_SIZE];  /* buffer size = 10; */
int items = 0; // number of items in the buffer.
int first = 0;
int last = 0; 
int consumed = 0;
int produced = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
 
void *producer();
void *consumer();

int main() {
   pthread_t CONS[C];
   pthread_t PROD[P];

  for (int i = 0; i < P; i++)
    pthread_create(&PROD[i], NULL, producer, NULL);
  for (int i = 0; i < C; i++)
    pthread_create(&CONS[i], NULL, consumer, NULL);


  for (int i = 0; i < P; i++)
    pthread_join(PROD[i], NULL);
  for (int i = 0; i < C; i++)
    pthread_join(CONS[i], NULL);

}

void put(int i){
  pthread_mutex_lock(&mutex);
  while(items == BUFFER_SIZE) { 
    pthread_cond_wait(&empty, &mutex); //Espera até que o buffer tenha espaço
  }
  buff[last] = i;
  items++; last++;

  if(last==BUFFER_SIZE) { last = 0; }  //Reinicio da fila circular
  if(items == 1) { 
    pthread_cond_broadcast(&fill); } //Chama a produção
  pthread_mutex_unlock(&mutex); 
}

void *producer() {
  int i = 0;
  printf("Produtor\n");
  while(produced <= NUM_ITEMS) {
    put(i);
    produced++;
    if (produced > NUM_ITEMS) break;
    printf("produced: %d\n", produced);
    if (produced == NUM_ITEMS)
        printf("Produziu todos os %d itens\n", produced);
  }
  items++;
  pthread_cond_broadcast(&fill);
  printf("producer finished\n");
  pthread_exit(NULL);
}

int get(){
  int result;
  pthread_mutex_lock(&mutex);
  while((items == 0) && (consumed < NUM_ITEMS)){
    printf("esvaziou\n");
    pthread_cond_wait(&fill, &mutex);
    printf("saiu\n");
  }
  result = buff[first];
  items--; first++;
  if(first==BUFFER_SIZE) { first = 0; }
  if(items == BUFFER_SIZE - 1){
    pthread_cond_broadcast(&empty); //Chama o consumo
  }
  pthread_mutex_unlock(&mutex);
  return result;
}

void *consumer() {
  int v;
  while (consumed <= NUM_ITEMS){
    v = get();
    consumed++;
    if (consumed > NUM_ITEMS) break;
    printf("consumed: %d\n", consumed);
    if (consumed == NUM_ITEMS) 
        printf("consumi todos os %d itens\n", consumed);
  }

  items--;
  pthread_cond_broadcast(&empty);
  printf("consumer finished\n");
  pthread_exit(NULL);
}