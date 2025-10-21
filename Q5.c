#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define HASH_SIZE 10

int HashTable[HASH_SIZE];

pthread_mutex_t BucketMutex[HASH_SIZE]; //cada bucket tem um mutex associado.

int Hash_function(int value) {return value % HASH_SIZE;} // Função Hash escolhida

void* Hash_Insert(void* ThreadParams)
{   
    int* value = ((int *) ThreadParams); //Recebimento do valor
    int index = Hash_function(*value);
    pthread_mutex_lock(&BucketMutex[index]); //Trava o mutex na região crítica
    if (HashTable[index] != 0) printf("ja tem um elemento, sobrescrevendo...\n");
    HashTable[index] = *value;

    printf("elemento %d adicionado com sucesso, saindo da r. crítica.\n", *value);
    pthread_mutex_unlock(&BucketMutex[index]);
    
    pthread_exit(NULL);
}   

void* Hash_Remove(void* ThreadParams)
{
    int* value = ((int *) ThreadParams); //Recebe o valor
    int index = Hash_function(*value);
    pthread_mutex_lock(&BucketMutex[index]); //Trava o mutex na região crítica
    HashTable[index] = 0;

    printf("elemento %d removido com sucesso. Saindo da r. Crítica. \n", *value);
    pthread_mutex_unlock(&BucketMutex[index]);
    
    pthread_exit(NULL);
}

void* Hash_Search(void* ThreadParams)
{
    int* value = ((int *) ThreadParams); //Recebe o valor
    int index = Hash_function(*value);
    pthread_mutex_lock(&BucketMutex[index]); //Trava o mutex na região crítica

    if (*value == HashTable[index]) printf("encontrei o elemento %d no index %d\n", *value, index);
    else printf("elemento %d não encontrado\n", *value);

    pthread_mutex_unlock(&BucketMutex[index]);
    
    pthread_exit(NULL);
}

int main()
{   
    //Menu
    char comando[10];

    printf("LISTA DE COMANDOS:\nEND\nINSERT\nREMOVE\nREAD\n");
    printf("oque você deseja fazer ?\n");

    scanf("%s", comando);

    while (strcmp(comando, "END") != 0)
    {
        if (strcmp(comando, "INSERT") == 0)
        {
            int value; printf("escreva o valor a ser escrito:\n"); scanf("%d", &value);
            pthread_t Insertion;
            int rc = pthread_create(&Insertion, NULL, Hash_Insert, (void *) &value);
            if (rc != 0) {printf("erro na criação da thread\n"); exit(-1);}

            pthread_join(Insertion, NULL);
        }
        else if (strcmp(comando, "REMOVE") == 0)
        {
            int value; printf("escreva o valor a ser removido:\n"); scanf("%d", &value);
            pthread_t Removal;
            int rc = pthread_create(&Removal, NULL, Hash_Remove, (void *) &value);
            if (rc != 0) {printf("erro na criação da thread\n"); exit(-1);}

            pthread_join(Removal, NULL);
        }
        else if (strcmp(comando, "READ") == 0)
        {
            int value; printf("escreva o valor a ser buscado:\n"); scanf("%d", &value);
            pthread_t Looking;
            int rc = pthread_create(&Looking, NULL, Hash_Search, (void *) &value);
            if (rc != 0) {printf("erro na criação da thread\n"); exit(-1);}

            pthread_join(Looking, NULL);
        }

        printf("oque você deseja fazer ?\n");
        scanf("%s", comando);
    }
    pthread_exit(NULL);
}