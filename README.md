# PThreadsSO - Solutions for Classic Concurrency Problems

This repository contains C implementations for a series of classic concurrency problems, using the Pthreads library. Each file addresses a different challenge, demonstrating the use of mutexes, condition variables, and synchronization strategies.

This project serves as excellent study material for anyone learning about Operating Systems and concurrent programming.

## 🚀 Technologies Used

* **Language:** C
* **Concurrency Library:** Pthreads (`pthread.h`)

## 📂 Project Structure

The repository is organized into 5 main problems:

### 1. Race Condition

* **File:** `race_condition.c`
* **Description:** A classic demonstration of a race condition. Multiple threads (`NUM_THREADS`) try to increment a global counter (`global_counter`) up to one million.
* **Solution:** A `pthread_mutex_t` (`mut`) is used to protect the critical section (the counter's read and increment), ensuring that only one thread can modify the variable at a time and announcing the "winning" thread.

### 2. Producer-Consumer Problem

* **File:** `produtor_consumidor.c`
* **Description:** Implements the Producer-Consumer problem with multiple producers (`P`), multiple consumers (`C`), and multiple buffers (`B`). Producers insert items, and consumers remove them.
* **Solution:** Each buffer is a circular queue protected by a mutex and two condition variables (`empty` and `fill`), which signal when the buffer is full or empty. This allows threads to wait efficiently without busy-looping.

### 3. Priority Print Queue

* **File:** `fila_impressao.c`
* **Description:** Simulates a printer that must process print jobs based on their priority. Multiple users (`NUM_USERS`) submit jobs with random priorities.
* **Solution:** The print queue is implemented as a **Max-Heap**. Access to the heap is controlled by a mutex (`queue_mutex`) and condition variables (`queue_not_empty`, `queue_not_full`) to manage the queue when it's full or empty.

### 4. Readers-Writers Problem

* **File:** `readers_writers.c`
* **Description:** Implements the Readers-Writers problem, where multiple reader threads (`N`) can access a shared resource (an array) simultaneously, but writer threads (`M`) require exclusive access.
* **Solution:** Uses a mutex and two condition variables (`writeNow_cond`, `readNow_cond`) to coordinate access. The system alternates priority between readers and writers to prevent starvation.

### 5. Concurrent Hash Table

* **File:** `hash_table_concorrente.c`
* **Description:** Implements a hash table that supports concurrent insertion, removal, and search operations by multiple threads.
* **Solution:** This demonstrates a **fine-grained locking** technique. Instead of a single global lock for the entire table, it uses an **array of mutexes** (`BucketMutex`), one for each *bucket* (linked list). This allows threads to operate on different buckets in parallel, significantly improving performance.

## ⚙️ How to Compile and Run

### Prerequisites

* C Compiler (e.g., `gcc`)
* Pthreads library (standard on Linux/macOS)

### Compilation

To compile, you **must** link the `pthread` library using the `-lpthread` flag.

Compile each problem individually. For example:

### Example for compiling the race condition
>gcc race_condition.c -o race_condition_app -lpthread

### Example for compiling the producer-consumer
>gcc produtor_consumidor.c -o produtor_consumidor_app -lpthread

### And so on...
>gcc fila_impressao.c -o fila_impressao_app -lpthread
>
>gcc readers_writers.c -o readers_writers_app -lpthread
>
>gcc hash_table_concorrente.c -o hash_table_app -lpthread


### Execution

After compiling, you can run each program:

>./race_condition_app
>
>./produtor_consumidor_app
>
>./fila_impressao_app
>
>./readers_writers_app
>
>./hash_table_app


Note: Most of these programs run in an infinite loop (or until a high limit is reached) to demonstrate concurrency. You will need to stop them manually using Ctrl+C.


✒️ Authors

@lucaasleal
@SillyGo
@AndreLJ
