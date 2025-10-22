#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

//Dados a definir
#define TAM_ARRAY 10
#define N 9                //  Número de Threads Leitoras
#define M 4                //  Número de Threads escritoras
#define WRITES_LIMIT 23    //  Tamanho de um ciclo de escritas
#define READS_LIMIT 41     //  Tamanho de um ciclo de leituras


//mutex e variáveis de condição
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t writeNow_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t readNow_cond = PTHREAD_COND_INITIALIZER;

//Dados como número de escritores e leitores no momento, além dos escritores esperando
int numReadersNow = 0;
int numWritersNow = 0;
int numWritersWaiting = 0;

//Leituras ou escritas já realizadas
int writesNow = 0;
int readsNow = 0;

//Booleano para informar de quem é a prioridade
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

    //Destruição dos mutexes e variáveis de condição
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&writeNow_cond);
    pthread_cond_destroy(&readNow_cond);

    return 0;
}

void *read(void* threadID){
    int* tid = ((int *) threadID);
    while(1){
        pthread_mutex_lock(&mutex);

        //Espera caso haja alguem escrevendo, ou seja a vez dos escritores, ou já tenha lido o limite N de leituras
        while(numWritersNow > 0 ||                                 
              writersPriority == 1 || readsNow >= READS_LIMIT){    
            pthread_cond_wait(&readNow_cond, &mutex);
        }

        //Entrou como leitor
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

        if(numReadersNow==0){ //Se não há nenhum leitor ativo, verifique se as leituras já terminaram
            if(readsNow >= READS_LIMIT){
                printf("------------------------------------------------------------------------\n");
                printf("%d READS FINISHED\n", readsNow);
                printf("------------------------------------------------------------------------\n");
                readsNow = 0;
                readersPriority = 0;                    //Tira a prioridade dos leitores
                writersPriority = 1;                    //Concede o próximo ciclo aos escritores
                pthread_cond_broadcast(&writeNow_cond); // Acorda todos os escritores
            } else {
                pthread_cond_broadcast(&readNow_cond);  // Acorda leitores se ainda não finalizaram as N leituras
            }
        } 
        pthread_mutex_unlock(&mutex);
    }
}

void *write(void* threadID){
    int* tid = ((int *) threadID);
    while(1){
        pthread_mutex_lock(&mutex);
        numWritersWaiting++; //Sinaliza que quer escrever

        //Espera caso haja leitores ativos, outro escritor ativo, ou seja a vez dos leitores
        while(numReadersNow > 0 ||                                   
              numWritersNow > 0 ||                                   
              readersPriority == 1 ){                                  
            pthread_cond_wait(&writeNow_cond, &mutex);
        }

        numWritersWaiting--; //Tira a sinalização, pois conseguiu entrar como escritor

        //Entra como escritor
        numWritersNow = 1;  
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
        if(writesNow == WRITES_LIMIT){  //Se já escreveu o limite M de escritas
            printf("------------------------------------------------------------------------\n");
            printf("%d WRITES FINISHED\n", writesNow);
            printf("------------------------------------------------------------------------\n");
            writesNow = 0;
            readersPriority = 1; //Concede a próxima vez aos leitores
            writersPriority = 0; //Tira a prioridade dos escritores
            pthread_cond_broadcast(&readNow_cond); // Acorda todos os leitores
        } else {
            pthread_cond_signal(&writeNow_cond);    // Acorda um escritor se ainda não finalizaram as M escritas
        }
        pthread_mutex_unlock(&mutex);
    }
}