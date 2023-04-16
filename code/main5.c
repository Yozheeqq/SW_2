#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#define TOBACCO 0
#define PAPER 1
#define MATCHES 2

sem_t *table_smoker_semaphores[3]; // семафоры для каждого из трех курильщиков
int table[3]; // компоненты на столе

void *table_mediator_thread(void *arg);
void *smoker_thread(void *arg);

int main() {
    srand(time(NULL));
    table_smoker_semaphores[0] = sem_open("tobacco_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[1] = sem_open("paper_sem", O_CREAT, 0644, 0);
    table_smoker_semaphores[2] = sem_open("matches_sem", O_CREAT, 0644, 0);
    pthread_t table_mediator_tid;
    pthread_create(&table_mediator_tid, NULL, table_mediator_thread, NULL);
    pthread_t smoker_tids[3];
    int smoker_args[3] = {TOBACCO, PAPER, MATCHES};
    for (int i = 0; i < 3; i++) {
        pthread_create(&smoker_tids[i], NULL, smoker_thread, &smoker_args[i]);
    }
    pthread_join(table_mediator_tid, NULL);
    for (int i = 0; i < 3; i++) {
        pthread_join(smoker_tids[i], NULL);
    }
    sem_close(table_smoker_semaphores[0]);
    sem_close(table_smoker_semaphores[1]);
    sem_close(table_smoker_semaphores[2]);
    sem_unlink("tobacco_sem");
    sem_unlink("paper_sem");
    sem_unlink("matches_sem");
    return 0;
}

void *table_mediator_thread(void *arg) {
    while (1) {
        int component1 = rand() % 3;
        int component2 = rand() % 3;
        while (component2 == component1) {
            component2 = rand() % 3;
        }
        table[0] = component1;
        table[1] = component2;
        printf("Медиатор дал компоненты %d и %d.\n", component1, component2);
        sem_post(table_smoker_semaphores[3 - component1 - component2]);
        sleep(3);
        sem_wait(table_smoker_semaphores[3 - component1 - component2]);  
        sleep(1);
    }
}

void *smoker_thread(void *arg) {
    int my_component = *(int*)arg;
    while (1) {
        sem_wait(table_smoker_semaphores[my_component]);
        sleep(2);
        printf("Курильщик %d скрутил сигарету и курит.\n", my_component);
        sem_post(table_smoker_semaphores[my_component]);
        sleep(2);
    }
}

