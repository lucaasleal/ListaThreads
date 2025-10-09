#include <stdio.h>
#include <pthread.h> 
#include <stdlib.h>

#define BUFFER_SIZE 10
#define NUM_ITEMS 50

#define C 8
#define P 3
#define B 5

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
   
   //Inicialização dos buffers
   for(int i = 0; i<B; i++){
    buffers[i].items = 0;
    buffers[i].first = 0;
    buffers[i].last = 0;
    pthread_mutex_init(&buffers[i].mutex, NULL);
    pthread_cond_init(&buffers[i].empty, NULL);
    pthread_cond_init(&buffers[i].fill, NULL);
  }

  //Criação das Threads
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

  while(buffer_atual->items == BUFFER_SIZE) { 
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
  while(1) {
    pthread_mutex_lock(&produced_mutex);

    if (produced >= NUM_ITEMS) {
      pthread_mutex_unlock(&produced_mutex);
      break;
    }

    int idx = rand()%B; //Random para escolha do buffer
    put(i, &buffers[idx]);  
    produced++;
    printf("Produzido no bfr %d: %d\n", idx, i++);
    pthread_mutex_unlock(&produced_mutex);

    if (produced == NUM_ITEMS){                         //Se produziu todos os itens, encerra
      printf("Produziu todos os %d itens\n", produced);
      pthread_mutex_unlock(&produced_mutex);
      break;
    }
  } 
  for(int i=0; i<B; i++){ // Acordar todas as threads (pode ser que algumas estejam dormindo)
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

  while((buffer_atual->items == 0) && (produced < NUM_ITEMS)){
    pthread_cond_wait(&buffer_atual->fill, &buffer_atual->mutex);
  }

  if (buffer_atual->items == 0 && produced >= NUM_ITEMS) {  
    pthread_mutex_unlock(&buffer_atual->mutex);
    return -1; // Sinal para o consumidor parar (Tudo já foi produzido e o buffer esvaziou, Deve-se ir para outro)
  }

  result = buffer_atual->data[buffer_atual->first];
  buffer_atual->first++; //Incrementa início da fila circular
  if(buffer_atual->first==BUFFER_SIZE) { buffer_atual->first = 0; } 
  buffer_atual->items--; 

  pthread_cond_broadcast(&buffer_atual->empty);
  pthread_mutex_unlock(&buffer_atual->mutex);
  return result;
}

void *consumer() {
  int value;

  while (1){
    pthread_mutex_lock(&consumed_mutex);

    if (consumed >= NUM_ITEMS) {   //Se já consumiu tudo, encerrar
      pthread_mutex_unlock(&consumed_mutex); 
      break;
    }

    pthread_mutex_unlock(&consumed_mutex);

    int idx = rand()%B; // Random para escolher buffer
    value = get(&buffers[idx]);
    if(value!=-1){ //Foi consumido um item do buffer
      pthread_mutex_lock(&consumed_mutex);
      consumed++;
      printf("Consumido do bfr %d: %d\n", idx, value);
      pthread_mutex_unlock(&consumed_mutex);
    }
  }

  for (int i = 0; i < B; i++) { //Acorda threads que possam estar dormindo
      pthread_mutex_lock(&buffers[i].mutex);
      pthread_cond_broadcast(&buffers[i].fill);
      pthread_mutex_unlock(&buffers[i].mutex);
  }
  printf("consumer finished\n");
  pthread_exit(NULL);
}