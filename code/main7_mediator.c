#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#define TOBACCO 0
#define PAPER 1
#define MATCHES 2

sem_t *table_smoker_semaphores[3];
sem_t *table_mutex;
int *table;

int main() {
    srand(time(NULL));
    table_mutex = sem_open("table_mutex", O_CREAT, 0644, 1);
    table_smoker_semaphores[0] = sem_open("tobacco_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[1] = sem_open("paper_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[2] = sem_open("matches_sem", O_CREAT, 0644, 0);
    int table_fd = shm_open("table_shm", O_RDWR, 0644);
    ftruncate(table_fd, 2 * sizeof(int));
    table = mmap(NULL, 2 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, table_fd, 0);
    while (1) {
        int component1 = rand() % 3;
        int component2 = rand() % 3;
        while (component2 == component1) {
            component2 = rand() % 3;
        }
        table[0] = component1;
        table[1] = component2;
        printf("Медиатор дал компоненты %d и %d.\n", component1, component2);
        // sem_post(table_mutex);
        sem_post(table_smoker_semaphores[3 - component1 - component2]);
        sleep(3);
        sem_wait(table_mutex);  
        sleep(1);
    }
    sem_close(table_mutex);
    sem_close(table_smoker_semaphores[0]);
    sem_close(table_smoker_semaphores[1]);
    sem_close(table_smoker_semaphores[2]);
    sem_unlink("tobacco_sem");
    sem_unlink("paper_sem");
    sem_unlink("matches_sem");
    sem_unlink("table_mutex");
}
