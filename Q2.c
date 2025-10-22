#include <stdio.h>
#include <pthread.h> 
#include <stdlib.h>

//ATENÇÂO
//O número de itens deve ser múltiplo de (buffer size x número de buffers)
// para garantir que todos os itens sejam consumidos corretamente, ou seja, os buffers possuirem
// mesma quantidade de itens produzidos e consumidos ao final da execução.

#define BUFFER_SIZE 10 //Tamanho de cada buffer
#define NUM_ITEMS 500 //Número total de itens a serem produzidos/consumidos

#define C 23 //Número de consumidores
#define P 29 //Número de produtores
#define B 20 //Número de buffers


//Essa struct representa um buffer que implementa uma fila circular
typedef struct{
  int data[BUFFER_SIZE];
  int idx;
  int items;
  //Informações da fila circular
  int first;
  int last; 
  //Mutex e variáveis de condição para controle de acesso (produtor/consumidor)
  pthread_mutex_t mutex;
  pthread_cond_t empty;
  pthread_cond_t fill;
} buffer;

//Contagem global de itens produzidos e consumidos
int consumed = 0;
int produced = 0;

void *producer();
void *consumer();

//mutexes para controle de consumo e produção
pthread_mutex_t produced_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumed_mutex = PTHREAD_MUTEX_INITIALIZER;

//Array de buffers
buffer buffers[B];

int main() {
  //criação das threads
   pthread_t CONS[C];
   pthread_t PROD[P];
   
   //Inicialização dos buffers
   for(int i = 0; i<B; i++){
    buffers[i].items = 0;
    buffers[i].first = 0;
    buffers[i].last = 0;
    //inicialização dos mutexes e variáveis de condição
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

  printf("Trabalho concluído. Produzido: %d, Consumido: %d\n", produced, consumed);

  //Destruição dos mutexes e variáveis de condição
  for (int i = 0; i < B; i++) {
    pthread_mutex_destroy(&buffers[i].mutex);
    pthread_cond_destroy(&buffers[i].empty);
    pthread_cond_destroy(&buffers[i].fill);
  }
  pthread_mutex_destroy(&produced_mutex);
  pthread_mutex_destroy(&consumed_mutex);
  
  return 0;

}

void put(int i, buffer *buffer_atual){
  pthread_mutex_lock(&buffer_atual->mutex);

  while(buffer_atual->items == BUFFER_SIZE) { //Se o buffer estiver cheio
    pthread_cond_wait(&buffer_atual->empty, &buffer_atual->mutex); //Espera até que o buffer tenha espaço
  }

  //Caso buffer esteja com espaço, insere o item na fila circular
  buffer_atual->data[buffer_atual->last] = i;
  buffer_atual->items++; 
  buffer_atual->last++;

  //Atualiza índice circular do buffer
  if(buffer_atual->last==BUFFER_SIZE) { buffer_atual->last = 0; }  //Reinicio da fila circular

  if(buffer_atual->items == 1) { 
    pthread_cond_signal(&buffer_atual->fill); } //Chama a produção se existe somente um item no buffer

  pthread_mutex_unlock(&buffer_atual->mutex); 
}

void *producer() {
  int i = 0;
  while(1) {
    pthread_mutex_lock(&produced_mutex);

    //Verifica se já produziu todos os itens
    if (produced >= NUM_ITEMS) {
      pthread_mutex_unlock(&produced_mutex);
      break;
    }

    //Produz um item
    i = produced;
    produced++;
    pthread_mutex_unlock(&produced_mutex);
    int idx = rand()%B; //Random para escolha do buffer
    put(i, &buffers[idx]);  
    printf("Produzido no bfr %d: %d\n", idx, i++);
    


    if (produced >= NUM_ITEMS){ //Se produziu todos os itens, encerra
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

  // Espera enquanto o buffer estiver vazio e ainda houver itens a serem produzidos
  while((buffer_atual->items == 0) && (produced < NUM_ITEMS)){
    pthread_cond_wait(&buffer_atual->fill, &buffer_atual->mutex);
  }

  if (buffer_atual->items == 0 && produced >= NUM_ITEMS) {  
    pthread_mutex_unlock(&buffer_atual->mutex);
    return -1; // Sinal para o consumidor parar (Tudo já foi produzido e o buffer esvaziou, Deve-se ir para outro)
  }

  //Retira o item do buffer (fila circular)
  result = buffer_atual->data[buffer_atual->first];
  buffer_atual->first++; //Incrementa início da fila circular
  if(buffer_atual->first==BUFFER_SIZE) { buffer_atual->first = 0; } //Reinicio da fila circular
  buffer_atual->items--; 

  //Se havia BUFFER_SIZE itens antes de retirar, agora  se pode produzir
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
    value = get(&buffers[idx]); //Tenta consumir do buffer escolhido
    if(value!=-1){ //Foi consumido um item do buffer
      pthread_mutex_lock(&consumed_mutex);
      consumed++;
      printf("Consumido do bfr %d: %d\n", idx, value);
      pthread_mutex_unlock(&consumed_mutex);
    }

    if (consumed >= NUM_ITEMS){   //Se consumiu todos os itens, encerra
      break;
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