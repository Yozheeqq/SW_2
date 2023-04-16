#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>

#define TOBACCO 0
#define PAPER 1
#define MATCHES 2

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s component\\n", argv[0]);
        return 1;
    }
    int my_component;
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

    // Получаем идентификатор семафоров
    int sem_id = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Failed to create semaphores");
        exit(1);
    }

    // Получаем идентификатор разделяемой памяти
    int shm_id = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        exit(1);
    }

    // Подключаемся к разделяемой памяти
    int *table = (int *) shmat(shm_id, NULL, 0);
    if ((void *) table == (void *) -1) {
        perror("Failed to attach shared memory");
        exit(1);
    }

    while (1) {      
        // Ждем, пока посредник положит компоненты на стол
        struct sembuf wait_mediator = {0, -1, SEM_UNDO};
        semop(sem_id, &wait_mediator, 1);

        // Проверяем, есть ли у нас необходимые компоненты
        if (table[0] != my_component && table[1] != my_component) {
            printf("Курильщик %d скрутил сигарету и курит.\n", my_component);

            // Курим
            sleep(2);

            // Оповещаем посредника, что закончили курить
            struct sembuf notify_mediator = {0, 1, SEM_UNDO};
            semop(sem_id, &notify_mediator, 1);
        } else {
        	printf("Компоненты %d %d не подходят", table[0], table[1]);
        }
    }

    // Отключаемся от разделяемой памяти
    shmdt(table);

    // Освобождаем разделяемую память
    shmctl(shm_id, IPC_RMID, NULL);

    // Освобождаем семафоры
    semctl(sem_id, 0, IPC_RMID, 0);

    return 0;
}
