#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>

#define TOBACCO 0
#define PAPER 1
#define MATCHES 2

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main() {
    srand(time(NULL));

    // Создаем семафоры
    int sem_id = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
    
    if (sem_id == -1) {
        perror("Failed to create semaphores");
        exit(1);
    }

    // Инициализируем семафоры
    union semun sem_val;
    sem_val.val = 0;
    semctl(sem_id, TOBACCO, SETVAL, sem_val);
    semctl(sem_id, PAPER, SETVAL, sem_val);
    semctl(sem_id, MATCHES, SETVAL, sem_val);

    // Создаем разделяемую память
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
        int component1 = rand() % 3;
        int component2 = rand() % 3;
        while (component2 == component1) {
            component2 = rand() % 3;
        }
        table[0] = component1;
        table[1] = component2;
        printf("Медиатор дал компоненты %d и %d.\n", component1, component2);
        // Оповещаем курильщиков
        struct sembuf notify_smokers = {0, 3, SEM_UNDO};
        semop(sem_id, &notify_smokers, 1);

        // Ждем, пока курильщик закончит
        struct sembuf wait_smokers = {0, 0, SEM_UNDO};
        semop(sem_id, &wait_smokers, 1);
        sleep(1);
    }

    // Отключаемся от разделяемой памяти
    shmdt(table);

    // Освобождаем разделяемую память
    shmctl(shm_id, IPC_RMID, NULL);

    // Освобождаем семафоры
    semctl(sem_id, 0, IPC_RMID, 0);
    return 0;
}
