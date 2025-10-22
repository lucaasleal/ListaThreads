#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10                                               // Tamanho máximo da fila de impressão
#define NUM_USERS 1                                                     // Número de usuários
#define FILES_PER_USER 0                                                // 0 se quiser arquivos infinitos
#define MAX_PRIORITY 10                                                 // Prioridade máxima dos arquivos

typedef struct {                                                        // Estrutura para representar um arquivo para impressão
    int priority;
    int id;
    char buffer[64];
} File;

File* files[MAX_QUEUE_SIZE];                                            // Heap para a fila de impressão
int queue_size = 0;                                                     // Tamanho atual da fila de impressão

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;                // Mutex para proteger o acesso à fila
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;              // Condicional para indicar que a fila não está vazia
pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;               // Condicional para indicar que a fila não está cheia

void swap(int a, int b) {                                               // Função para trocar dois elementos na heap
    File* temp = files[a];
    files[a] = files[b];
    files[b] = temp;
}

void heapify_up(int index) {                                            // Função para manter a propriedade do heap ao inserir um novo arquivo               
    while (index > 0 && files[index] -> priority > files[(index - 1) / 2] -> priority) {
        swap(index, (index - 1) / 2);
        index = (index - 1) / 2;
    }
}

void heapify_down(int index) {                                          // Função para manter a propriedade do heap ao remover um arquivo
    int max = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < queue_size && files[left] -> priority > files[max]->priority) {
        max = left;
    }
    if (right < queue_size && files[right] -> priority > files[max] -> priority) {
        max = right;
    }
    if (max != index) {
        swap(index, max);
        heapify_down(max);
    }
}

void* printer(void* ThreadID) {                                         // Função da thread da impressora               
    while (1) {                 
        pthread_mutex_lock(&queue_mutex);                               // Bloqueia o mutex para acessar a fila
        while (queue_size == 0) {
            printf("Impressora ociosa, aguardando arquivos...\n");      // Espera até que haja arquivos na fila
            pthread_cond_wait(&queue_not_empty, &queue_mutex);         
        }

        File* file_2_print = files[0];                                  // Remove o arquivo de maior prioridade da fila         
        files[0] = files[queue_size - 1];                               // Move o último arquivo para a raiz
        queue_size--;                                                   // Decrementa o tamanho da fila                     
        heapify_down(0);                                                // Restaura a propriedade do heap
                                                                        // -> Algoritmo de remoção de heap de máximo

        pthread_cond_signal(&queue_not_full);                           // Sinaliza que a fila não está cheia
        pthread_mutex_unlock(&queue_mutex);                             // Desbloqueia o mutex  

        printf("Imprimindo arquivo ID: %d com prioridade: %d\n", file_2_print->id, file_2_print->priority);
        printf("Conteúdo: %s\n", file_2_print->buffer);
        free(file_2_print);
    }
}

void* user(void* ThreadID) {                                            // Função da thread do usuário              
    int id = *((int*)ThreadID);
    for (int i = 0; i < FILES_PER_USER || FILES_PER_USER == 0; ++i) {   // Loop para enviar arquivos para impressão por um usuário
        File* new_file = (File*)malloc(sizeof(File));                   // Aloca memória para um novo arquivo
        new_file->id = id;                                              // Define o ID do usuário
        new_file->priority = rand() % (MAX_PRIORITY + 1);               // Define uma prioridade aleatória de valor entre 0 e MAX_PRIORITY
        
        for(int j = 0; j < 63; ++j) {                                   // Preenche o buffer com caracteres aleatórios    
            new_file->buffer[j] = 'A' + (rand() % 26);
        }
        new_file->buffer[63] = '\0';                                    // Importante: sinaliza que é o fim de uma string de conteúdo

        pthread_mutex_lock(&queue_mutex);
        while (queue_size == MAX_QUEUE_SIZE) {
            printf("Fila cheia. Usuário %d aguardando para enviar arquivo %d...\n", id, i);
            pthread_cond_wait(&queue_not_full, &queue_mutex);
        }

        files[queue_size] = new_file;
        queue_size++;
        heapify_up(queue_size - 1);

        printf("Usuário %d enviou um arquivo com prioridade %d\n", id, new_file->priority); //Caso o arquivo tenha sido adicionado com sucesso

        pthread_cond_signal(&queue_not_empty);
        pthread_mutex_unlock(&queue_mutex);
    }
    return NULL;
}

int main() {
    //Definição e criação das threads
    pthread_t printer_thread;
    pthread_t user_threads[NUM_USERS];
    int user_ids[NUM_USERS];

    pthread_create(&printer_thread, NULL, printer, NULL);
    for (int i = 0; i < NUM_USERS; ++i) {
        user_ids[i] = i + 1;
        pthread_create(&user_threads[i], NULL, user, &user_ids[i]);
    }

    for (int i = 0; i < NUM_USERS; ++i) {
        pthread_join(user_threads[i], NULL);
    }

    pthread_join(printer_thread, NULL);

    return 0;
}