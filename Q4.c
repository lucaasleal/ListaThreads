#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM_ARRAY 10
#define N 15 //  Número de Threads Leitoras
#define M 13 //  Número de Threads escritoras
#define WRITES_LIMIT 10
#define READS_LIMIT 30

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t writeNow_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t readNow_cond = PTHREAD_COND_INITIALIZER;

int numReadersNow = 0;
int numWritersNow = 0;
int numWritersWaiting = 0;

int writesNow = 0;
int readsNow = 0;
int writersPriority = 0;
int readersPriority = 0;

int arr[TAM_ARRAY];

void *read();
void *write();

int main(){
    //Inicialização das threads
    pthread_t readers[N];
    pthread_t writers[M];
    int readersThreadsID[N];
    int writersThreadsID[M];

    for (int i = 0; i < N; i++){
        readersThreadsID[i] = i;
        pthread_create(&readers[i], NULL, read, &readersThreadsID[i]);
    }
    for (int i = 0; i < M; i++){
        writersThreadsID[i] = i;
        pthread_create(&writers[i], NULL, write, &writersThreadsID[i]);
    }

    for (int i = 0; i < N; i++){
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < M; i++){
        pthread_join(writers[i], NULL);
    }

    return 0;
}

void *read(void* threadID){
    int* tid = ((int *) threadID);
    while(1){
        pthread_mutex_lock(&mutex);
        while(numWritersNow > 0 ||                                 // Escritor ativo
              writersPriority == 1 ||                                   // É a vez dos escritores
              (numWritersWaiting > 0 && readsNow >= READS_LIMIT)){ //Espera enquanto alguém escreve ou está querendo escrever
            pthread_cond_wait(&readNow_cond, &mutex);
        }

        if(readersPriority == 1 || readsNow>=READS_LIMIT){
            readsNow = 0;
            readersPriority = 0;
        }
        
        numReadersNow++;
        readsNow++;
        pthread_mutex_unlock(&mutex);
        
        //Leitura do dado
        int idx = rand()%TAM_ARRAY;
        int data = arr[idx];
        printf("Value %d read in arr[%d] by reader %d\n", data, idx, *tid);
        //Liberação para outro leitor
        pthread_mutex_lock(&mutex);
        numReadersNow--;
        if(numReadersNow==0){
            if(readsNow == READS_LIMIT){
                printf("%d reads finished\n", readsNow);
                readsNow = 0;
                writersPriority = 1;
                readersPriority = 0;
                pthread_cond_broadcast(&writeNow_cond); // Acorda todos os escritores
            } else {
                pthread_cond_broadcast(&readNow_cond);  // Acorda leitores
                pthread_cond_signal(&writeNow_cond);    // Acorda um escritor
            }
        } else {
            pthread_cond_signal(&writeNow_cond);    // Acorda um escritor
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *write(void* threadID){
    int* tid = ((int *) threadID);
    while(1){
        pthread_mutex_lock(&mutex);
        numWritersWaiting++; //Sinaliza que quer escrever

        while(numReadersNow > 0 ||                                    // Leitores ativos
              numWritersNow > 0 ||                                    // Outro escritor ativo
              readersPriority == 1 ||                                   // É a vez dos leitores
              (numReadersNow > 0 && writesNow >= WRITES_LIMIT)){ //Espera enquanto alguém lê ou escreve
            pthread_cond_wait(&writeNow_cond, &mutex);
        }

        if(writersPriority == 1 || writesNow>=READS_LIMIT){
            writesNow = 0;
            writersPriority = 0;
        }

        numWritersWaiting--; //Tira a sinalização, pois conseguiu entrar como escritor
        numWritersNow = 1;  //Entra como escritor
        writesNow++;
        pthread_mutex_unlock(&mutex);

        //Escrita do dado
        int idx = rand()%TAM_ARRAY;
        int data = rand()%1000;
        arr[idx] = data;
        printf("Value %d written in arr[%d] by writer %d\n", data, idx, *tid);
        
        //Liberação para outro escritor ou leitores
        pthread_mutex_lock(&mutex);
        numWritersNow = 0;
        if(writesNow >= WRITES_LIMIT){
            printf("%d writes finished\n", writesNow);
            writersPriority = 0;
            readersPriority = 1;
            pthread_cond_broadcast(&readNow_cond); // Acorda todos os leitores
        } else {
            pthread_cond_broadcast(&readNow_cond);  // Acorda leitores
            pthread_cond_broadcast(&writeNow_cond);    // Acorda um escritor
        }
        pthread_mutex_unlock(&mutex);
    }
}