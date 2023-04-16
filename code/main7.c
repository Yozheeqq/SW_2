#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#define TOBACCO 0
#define PAPER 1
#define MATCHES 2

sem_t *table_smoker_semaphores[3];
sem_t *table_mutex;
int *table;
int my_component;

void *table_mediator_thread(void *arg);
void *smoker_thread(void *arg);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s component\\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "tobacco") == 0) {
        my_component = TOBACCO;
    }
    else if (strcmp(argv[1], "paper") == 0) {
        my_component = PAPER;
    }
    else if (strcmp(argv[1], "matches") == 0) {
        my_component = MATCHES;
    }
    else {
        printf("Invalid component: %s\\n", argv[1]);
        return 1;
    }
    table_mutex = sem_open("table_mutex", O_CREAT, 0644, 1);
    table_smoker_semaphores[0] = sem_open("tobacco_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[1] = sem_open("paper_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[2] = sem_open("matches_sem", O_CREAT, 0644, 0);
    int table_fd = shm_open("table_shm", O_RDWR | O_CREAT, 0644);
    ftruncate(table_fd, 2 * sizeof(int));
    table = mmap(NULL, 2 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, table_fd, 0);
    while (1) {
        sem_wait(table_smoker_semaphores[my_component]);
        sem_wait(table_mutex);
        if (table[0] != my_component && table[1] != my_component) {
            printf("Smoker with component %d got the components and is smoking\\n", my_component);
            table[0] = table[1] = -1;
            sem_post(table_mutex);
            // курим
            sleep(1);
            // закончили курить
            sem_post(table_smoker_semaphores[table[2]]);
        }
        else {
            sem_post(table_mutex);
        }
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#define TOBACCO 0
#define PAPER 1
#define MATCHES 2

sem_t *table_smoker_semaphores[3];
sem_t *table_mutex;
int *table;

void *table_mediator_thread(void *arg);
void *smoker_thread(void *arg);

int main() {
    srand(time(NULL));
    table_mutex = sem_open("table_mutex", O_CREAT, 0644, 1);
    table_smoker_semaphores[0] = sem_open("tobacco_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[1] = sem_open("paper_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[2] = sem_open("matches_sem", O_CREAT, 0644, 0);
    int table_fd = shm_open("table_shm", O_RDWR | O_CREAT, 0644);
    ftruncate(table_fd, 2 * sizeof(int));
    table = mmap(NULL, 2 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, table_fd, 0);
    while (1) {
        sem_wait(table_mutex);
        int component1 = rand() % 3;
        int component2 = rand() % 3;
        while (component2 == component1) {
            component2 = rand() % 3;
        }
        table[0] = component1;
        table[1] = component2;
        printf("Mediator put %d and %d on the table\\n", component1, component2);
        sem_post(table_smoker_semaphores[component1]);
        sem_post(table_smoker_semaphores[component2]);
        sem_post(table_mutex);
        sleep(1); // пауза перед следующей итерацией
    }
}

