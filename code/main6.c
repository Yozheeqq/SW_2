#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

#define NUM_COMPONENTS 3
#define NUM_SMOKERS 3

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

struct shared_mem {
    int component1;
    int component2;
};

int sem_id, shm_id;
void* shm_ptr;
struct shared_mem* shared_memory;

void smoker(int smoker_id, int component) {
    while (1) {
        // Ждем, пока посредник положит компоненты на стол
        struct sembuf wait_mediator = {0, -1, SEM_UNDO};
        semop(sem_id, &wait_mediator, 1);

        // Проверяем, есть ли у нас необходимые компоненты
        if (shared_memory->component1 != component && shared_memory->component2 != component &&
            shared_memory->component1 != 0 && shared_memory->component2 != 0) {
            shared_memory->component1 = 0;
            shared_memory->component2 = 0;
            printf("Курильщик %d скрутил сигарету и курит.\n", smoker_id);

            // Курим
            sleep(2);

            // Оповещаем посредника, что закончили курить
            struct sembuf notify_mediator = {0, 1, SEM_UNDO};
            semop(sem_id, &notify_mediator, 1);
        }
    }
}

void mediator() {
    while (1) {
        // Положим на стол два случайных компонента
        shared_memory->component1 = rand() % 3 + 1;
        do {
        	shared_memory->component2 = rand() % 3 + 1;
        } while (shared_memory->component1 == shared_memory->component2);

        printf("Медиатор дал компоненты %d и %d.\n", shared_memory->component1, shared_memory->component2);

        // Оповещаем курильщиков
        struct sembuf notify_smokers = {0, NUM_SMOKERS, SEM_UNDO};
        semop(sem_id, &notify_smokers, 1);

        // Ждем, пока курильщик закончит
        struct sembuf wait_smokers = {0, 0, SEM_UNDO};
        semop(sem_id, &wait_smokers, 1);
        sleep(1);
    }
}

int main() {
    // Создаем и инициализируем cемафор
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    union semun sem_union;
    sem_union.val = 0;
    semctl(sem_id, 0, SETVAL, sem_union);

    // Создаем и инициализируем разделяемую память
    shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_mem), IPC_CREAT | 0666);
    shm_ptr = shmat(shm_id, NULL, 0);
    shared_memory = (struct shared_mem*) shm_ptr;

    // Инициализируем генератор случайных чисел
    srand(time(NULL));

    // Создаем процессы-курильщиков
    for (int i = 0; i < NUM_SMOKERS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("Ошибка при создании процесса курильщика\n");
            return 1;
        } else if (pid == 0) {
            smoker(i + 1, i + 1);
            return 0;
        }
    }

    // Создаем процесс-посредник
    pid_t pid = fork();
    if (pid < 0) {
        printf("Ошибка при создании процесса посредника\n");
        return 1;
    } else if (pid == 0) {
        mediator();
        return 0;
    }

    // Ждем завершения дочерних процессов
    for (int i = 0; i < NUM_SMOKERS + 1; i++) {
        wait(NULL);
    }

    // Освобождаем ресурсы
    semctl(sem_id, 0, IPC_RMID, sem_union);
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
