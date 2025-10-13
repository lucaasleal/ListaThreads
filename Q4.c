#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM_ARRAY 10
#define N 2 //  Número de Threads Leitoras
#define M 1 //  Número de Threads escritoras

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t writingNow = PTHREAD_COND_INITIALIZER;
pthread_cond_t readingNow = PTHREAD_COND_INITIALIZER;

int numReadersNow = 0;
int numWritersNow = 0;
int numWritersWaiting = 0;
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

    for (int i = 0; i < N; i++)
        pthread_join(&readers[i], NULL);
    for (int i = 0; i < M; i++)
        pthread_join(&writers[i], NULL);

    return 0;
}

void *read(void* threadID){
    int* tid = ((int *) threadID);
    while(1){
        pthread_mutex_lock(&mutex);
        while(numWritersNow>0 || numWritersWaiting>0){ //Espera enquanto alguém escreve ou está querendo escrever
            pthread_cond_wait(&writingNow, &mutex);
        }
        numReadersNow++;
        pthread_mutex_unlock(&mutex);
        
        //Leitura do dado
        int idx = rand()%TAM_ARRAY;
        int data = arr[idx];
        printf("Value %d readed in arr[%d] by reader %d\n", data, idx, *tid);

        //Liberação para outro leitor
        pthread_mutex_lock(&mutex);
        numReadersNow--;
        if(numReadersNow==0)
            pthread_cond_signal(&readingNow); //Acorda o primeiro escritor
        pthread_mutex_unlock(&mutex);
    }
}

void *write(void* threadID){
    int* tid = ((int *) threadID);
    while(1){
        pthread_mutex_lock(&mutex);
        numWritersWaiting++; //Sinaliza que quer escrever

        while(numReadersNow>0 || numWritersNow>0){ //Espera enquanto alguém lê ou escreve
            pthread_cond_wait(&readingNow, &mutex);
        }
        numWritersNow = 1;  //Entra como escritor
        numWritersWaiting--; //Tira a sinalização, pois conseguiu entrar como escritor
        pthread_mutex_unlock(&mutex);

        //Escrita do dado
        int idx = rand()%TAM_ARRAY;
        int data = rand();
        arr[idx] = data;
        printf("Value %d writed in arr[%d] by writer %d\n", data, idx, *tid);

        //Liberação para outro escritor ou leitores
        pthread_mutex_lock(&mutex);
        numWritersNow = 0;
        if(numWritersWaiting>0){    //Caso alguém ainda queira escrever
            pthread_cond_signal(&readingNow); //Acorda o próximo escritor
        } else {
            pthread_cond_broadcast(&writingNow); //Acorda os leitores
        }
        pthread_mutex_unlock(&mutex);
    }
}