#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define HASH_SIZE 10
#define WRITES_NUM 50
#define READS_NUM 50
#define SEARCHES_NUM 50
#define NUM_THREADS 10 // número de threads simultâneas para cada operação


/*
    Implementação de uma tabela hash com operações concorrentes de inserção, remoção e busca.
    Cada bucket da tabela é protegido por um mutex para garantir a integridade dos dados durante as operações concorrentes.
    Utilizamos um ciclo de tamanho escolhido pelo usuário para escrever, buscar e remover elementos da tabela hash.
 */

 
 // Contadores para operações realizadas
int writes_done = 0;
int reads_done = 0;
int searches_done = 0;

typedef struct Node {
    int value;
    struct Node* next;
} Node;

// Tabela hash composta por listas encadeadas
Node* HashTable[HASH_SIZE];
pthread_mutex_t BucketMutex[HASH_SIZE]; //Mutex pra cada região crítica

int Hash_function(int value) {
    return value % HASH_SIZE;
}

// Inserção concorrente
void* Hash_Insert(void *ThreadID) {   
    while(writes_done < WRITES_NUM){
        writes_done++;
        int value = rand() % 100;; 
        int index = Hash_function(value);

        pthread_mutex_lock(&BucketMutex[index]); // região crítica

        //Lógica para inserção do item na lista encadeada do bucket
        Node* newNode = malloc(sizeof(Node));
        newNode->value = value;
        newNode->next = HashTable[index];
        HashTable[index] = newNode;

        printf("[INSERÇÃO] Tid %d: valor %d inserido no bucket %d.\n", ThreadID, value, index); //

        pthread_mutex_unlock(&BucketMutex[index]);
    }
}

// Remoção concorrente
void* Hash_Remove(void *ThreadID) {
    while(searches_done < SEARCHES_NUM){
        searches_done++;
        int value = rand() % 100;
        int index = Hash_function(value);

        pthread_mutex_lock(&BucketMutex[index]); // região crítica

        //Lógica para remoção do item na lista encadeada do bucket
        Node* curr = HashTable[index];
        Node* prev = NULL;
        while (curr != NULL) {
            if (curr->value == value) {
                if (prev == NULL) HashTable[index] = curr->next;
                else prev->next = curr->next;

                free(curr); //Libera memória
                printf("[REMOÇÃO] Tid %d: valor %d removido do bucket %d.\n", ThreadID, value, index);
                pthread_mutex_unlock(&BucketMutex[index]); //Se achou, libera o mutex e sai
                break;
            }
            prev = curr;
            curr = curr->next; 
        }

        printf("[REMOÇÃO] Tid %d: valor %d não encontrado no bucket %d.\n", ThreadID, value, index);
        pthread_mutex_unlock(&BucketMutex[index]);
    }
    
}

// Busca concorrente
void* Hash_Search(void *ThreadID) {
    while(reads_done < READS_NUM){
        reads_done++;
        int value = rand() % 100;
        int index = Hash_function(value);

        pthread_mutex_lock(&BucketMutex[index]); // região crítica

        //Lógica para busca do item na lista encadeada do bucket
        Node* curr = HashTable[index];
        while (curr != NULL) {
            if (curr->value == value) {
                printf("[BUSCA] Tid %d: valor %d encontrado no bucket %d.\n", ThreadID, value, index);
                pthread_mutex_unlock(&BucketMutex[index]);
                pthread_exit(NULL);
            }
            curr = curr->next;
        }

        printf("[BUSCA] Tid %d: valor %d não encontrado no bucket %d.\n", ThreadID, value, index);

        pthread_mutex_unlock(&BucketMutex[index]);
    }
}

// Função para imprimir o estado atual da tabela
void Print_HashTable() {
    printf("\n--- Estado atual da Hash Table ---\n");
    for (int i = 0; i < HASH_SIZE; i++) {
        printf("[%d]: ", i);
        Node* curr = HashTable[i];
        while (curr != NULL) {
            printf("%d -> ", curr->value);
            curr = curr->next;
        }
        printf("NULL\n");
    }
    printf("----------------------------------\n\n");
}

int main() {
    // Inicializa mutexes e tabela
    for (int i = 0; i < HASH_SIZE; i++) {
        pthread_mutex_init(&BucketMutex[i], NULL);
        HashTable[i] = 0;
    }

    pthread_t threads[NUM_THREADS * 3]; // inserção, busca e remoção
    int thread_count = 0;

    int Hash_Inserts_IDs[NUM_THREADS];
    int Hash_Search_IDs[NUM_THREADS];
    int Hash_Remove_IDs[NUM_THREADS];
    
    // Cria threads 
    for (int i = 0; i < NUM_THREADS; i++) {
        Hash_Inserts_IDs[i] = i;
        pthread_create(&threads[thread_count++], NULL, Hash_Insert, Hash_Inserts_IDs[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        Hash_Search_IDs[i] = i;
        pthread_create(&threads[thread_count++], NULL, Hash_Search, Hash_Search_IDs[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        Hash_Remove_IDs[i] = i;
        pthread_create(&threads[thread_count++], NULL, Hash_Remove, Hash_Remove_IDs[i]);
    }
    
    for (int i = 0; i < thread_count; i++) { 
        pthread_join(threads[i], NULL); 
    }

    // Imprime o estado final da tabela
    Print_HashTable();

    // Limpeza
    for (int i = 0; i < HASH_SIZE; i++) {
        Node* curr = HashTable[i];
        while (curr != NULL) {
            Node* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }

    pthread_exit(NULL);
}
