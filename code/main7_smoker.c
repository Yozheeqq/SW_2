#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>

#define TOBACCO 0
#define PAPER 1
#define MATCHES 2

sem_t *table_smoker_semaphores[3];
sem_t *table_mutex;
int *table;
int my_component;


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s component\n", argv[0]);
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
        printf("Invalid component: %s\n", argv[1]);
        return 1;
    }
    table_mutex = sem_open("table_mutex", O_CREAT, 0644, 1);
    table_smoker_semaphores[0] = sem_open("tobacco_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[1] = sem_open("paper_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[2] = sem_open("matches_sem", O_CREAT, 0644, 0);
    int table_fd = shm_open("table_shm", O_RDWR, 0644);
    ftruncate(table_fd, 2 * sizeof(int));
    table = mmap(NULL, 2 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, table_fd, 0);
    while (1) {
        sem_wait(table_smoker_semaphores[my_component]);
        sleep(2);
        if (table[0] != my_component && table[1] != my_component) {
        	printf("Курильщик %d скрутил сигарету и курит.\n", my_component);
        	sem_post(table_mutex);
        	sleep(2);
        }
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
