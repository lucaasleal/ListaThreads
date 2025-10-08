#include <stdio.h>
#include <pthread.h> 
#include <stdlib.h>

#define BUFFER_SIZE 10
#define NUM_ITEMS 5000

#define C 50
#define P 50
#define B 10

typedef struct{
  int data[BUFFER_SIZE];
  int idx;
  int items;
  int first;
  int last; 
  pthread_mutex_t mutex;
  pthread_cond_t empty;
  pthread_cond_t fill;
} buffer;

int consumed = 0;
int produced = 0;

void *producer();
void *consumer();

pthread_mutex_t produced_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumed_mutex = PTHREAD_MUTEX_INITIALIZER;

buffer buffers[B];

int main() {
   pthread_t CONS[C];
   pthread_t PROD[P];
   for(int i = 0; i<B; i++){
    buffers[i].items = 0;
    buffers[i].first = 0;
    buffers[i].last = 0;
    pthread_mutex_init(&buffers[i].mutex, NULL);
    pthread_cond_init(&buffers[i].empty, NULL);
    pthread_cond_init(&buffers[i].fill, NULL);
  }

  for (int i = 0; i < P; i++)
    pthread_create(&PROD[i], NULL, producer, NULL);
  for (int i = 0; i < C; i++)
    pthread_create(&CONS[i], NULL, consumer, NULL);


  for (int i = 0; i < P; i++)
    pthread_join(PROD[i], NULL);
  for (int i = 0; i < C; i++)
    pthread_join(CONS[i], NULL);

}

void put(int i, buffer *buffer_atual){
  pthread_mutex_lock(&buffer_atual->mutex);
  while(&buffer_atual->items == BUFFER_SIZE) { 
    pthread_cond_wait(&buffer_atual->empty, &buffer_atual->mutex); //Espera até que o buffer tenha espaço
  }
  buffer_atual->data[buffer_atual->last] = i;
  buffer_atual->items++; 
  buffer_atual->last++;

  if(buffer_atual->last==BUFFER_SIZE) { buffer_atual->last = 0; }  //Reinicio da fila circular
  if(buffer_atual->items == 1) { 
    pthread_cond_broadcast(&buffer_atual->fill); } //Chama a produção
  pthread_mutex_unlock(&buffer_atual->mutex); 
}

void *producer() {
  int i = 0;
  printf("Produtor\n");
  while(1) {
    pthread_mutex_lock(&produced_mutex);
      if (produced >= NUM_ITEMS) {
        pthread_mutex_unlock(&produced_mutex);
        break;
      }
      int idx = rand()%B;
      put(i, &buffers[idx]);
      produced++;
      printf("produced by bf %d: %d\n", idx, produced);
      pthread_mutex_unlock(&produced_mutex);
      if (produced == NUM_ITEMS){
        printf("Produziu todos os %d itens\n", produced);
        pthread_mutex_unlock(&produced_mutex);
        break;
      }
  } 
  for(int i=0; i<B; i++){
    pthread_mutex_lock(&buffers[i].mutex);
    pthread_cond_broadcast(&buffers[i].fill);
    pthread_mutex_unlock(&buffers[i].mutex);
  }
  printf("producer finished\n");
  pthread_exit(NULL);
}

int get(buffer *buffer_atual){
  int result;
  pthread_mutex_lock(&buffer_atual->mutex);
  while((&buffer_atual->items == 0) && (produced < NUM_ITEMS)){
    pthread_cond_wait(&buffer_atual->fill, &buffer_atual);
  }

  if (buffer_atual->items == 0 && produced >= NUM_ITEMS) {
    pthread_mutex_unlock(&buffer_atual->mutex);
    return -1; // Sinal para o consumidor parar
  }

  if(buffer_atual->first==BUFFER_SIZE) { buffer_atual->first = 0; }
  if(buffer_atual->items == BUFFER_SIZE - 1){
    pthread_cond_broadcast(&buffer_atual->empty); //Chama o consumo
  }
  result = buffer_atual->data[buffer_atual->first];
  buffer_atual->items--; 
  buffer_atual->first++;
  pthread_mutex_unlock(&buffer_atual->mutex);
  return result;
}

void *consumer() {
  int v;
  while (consumed <= NUM_ITEMS){
    int idx = rand()%B;
    v = get(&buffers[idx]);
    pthread_mutex_lock(&consumed_mutex);
    consumed++;
    pthread_mutex_unlock(&consumed_mutex);
    if (consumed > NUM_ITEMS) break;
    printf("consumed by bf %d: %d\n", idx, consumed);
    if (consumed == NUM_ITEMS) 
        printf("consumi todos os %d itens\n", consumed);
  }

  for (int i = 0; i < B; i++) {
      pthread_mutex_lock(&buffers[i].mutex);
      pthread_cond_broadcast(&buffers[i].fill);
      pthread_mutex_unlock(&buffers[i].mutex);
  }
  printf("consumer finished\n");
  pthread_exit(NULL);
}