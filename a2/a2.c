#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

// initializez global semafoarele pentru sincronizare

sem_t semafor_thr3_3_start;
sem_t semafor_thr3_4_end;
sem_t semafor_thr7_max5;

// #define NUM_THREADS 48
// #define MAX_RUNNING_THREADS 5

// // Structura folosita pentru bariera thread-urilor
// typedef struct
// {
//     int count;
//     int nr_threads_active;
//     sem_t mutex;
//     sem_t bariera;
// } Bariera;

// // Variabila globala pentru barieră
// Bariera bariera;

// void barrier_init(int nr_threads_active) //functia de initializare
// {
//     bariera.count = 0;
//     bariera.nr_threads_active = nr_threads_active;
//     sem_init(&bariera.mutex, 0, 1);
//     sem_init(&bariera.bariera, 0, 0);
// }

// void barrier_wait()
// {
//     sem_wait(&bariera.mutex);
//     bariera.count++; //marim contorul
//     sem_post(&bariera.mutex);

//     if (bariera.count == bariera.nr_threads_active)
//     {
//         for (int i = 0; i < bariera.nr_threads_active - 1; ++i) {
//             sem_post(&bariera.bariera);
//         }
//     }

//     sem_wait(&bariera.bariera);
//     sem_post(&bariera.bariera);
// }

// Funcția thread pentru P7
void *thread_fn_p7(void *arg)
{
    int thread_num = *(int *)arg;

    // if (thread_num == 0) {
    //     info(BEGIN, 7, thread_num);
    //     barrier_wait(); // Așteaptă la barieră pentru a se asigura că toate celelalte thread-uri au început
    // } else
    // {
    //     info(BEGIN, 7, thread_num);
    //     barrier_wait(); // Așteaptă la barieră
    //     info(END, 7, thread_num);
    // }

    sem_wait(&semafor_thr7_max5);
    info(BEGIN, 7, thread_num);
    info(END, 7, thread_num);
    sem_post(&semafor_thr7_max5);
    return NULL;
}

void *thread_fn_p3(void *arg)
{
    int nr_thread = *(int *)arg;

    if (nr_thread == 4)
    {
        sem_wait(&semafor_thr3_3_start);
    }
    info(BEGIN, 3, nr_thread);

    if (nr_thread == 3)
    {
        sem_post(&semafor_thr3_3_start);
    }

    if (nr_thread == 3)
    {
        sem_wait(&semafor_thr3_4_end);
    }

    info(END, 3, nr_thread);
    if (nr_thread == 4)
    {
        sem_post(&semafor_thr3_4_end);
    }

    return NULL;
}

int main()
{
    init();
    info(BEGIN, 1, 0); // inceperea primului proces P1

    // initializam procesele
    pid_t pID2, pID3, pID4, pID5, pID6, pID7, pID8;

    // deschidem semafoarele initializate

    pID2 = fork();
    if (pID2 == 0) // Procesul 2 , porneste toate celelalte procese
    {
        info(BEGIN, 2, 0); // inceperea procesului 2

        pID3 = fork(); // aici luam procesul 3 si il incepem
        if (pID3 == 0)
        {
            info(BEGIN, 3, 0);
            sem_init(&semafor_thr3_3_start, 0, 0);
            sem_init(&semafor_thr3_4_end, 0, 0);

            //------ crearea thread-urilor procesului 3------//
            pthread_t threads[4];
            int nr_threads[4] = {1, 2, 3, 4};

            for (int i = 0; i < 4; i++)
            {
                pthread_create(&threads[i], NULL, thread_fn_p3, &nr_threads[i]);
            }
            for (int i = 0; i < 4; i++)
            {
                pthread_join(threads[i], NULL);
            }

            pID4 = fork(); // luam 4 si incepem
            if (pID4 == 0)
            {
                info(BEGIN, 4, 0);
                info(END, 4, 0); // final 4
                _exit(0);
            }
            waitpid(pID4, NULL, 0);

            info(END, 3, 0); // final 3
            _exit(0);
        }
        waitpid(pID3, NULL, 0);

        pID5 = fork(); // luam 5 si incepem
        if (pID5 == 0)
        {
            info(BEGIN, 5, 0);

            pID6 = fork();
            if (pID6 == 0)
            {
                info(BEGIN, 6, 0); // inceput 6
                info(END, 6, 0);   // final 6
                _exit(0);
            }
            waitpid(pID6, NULL, 0);

            info(END, 5, 0); // final 5
            _exit(0);
        }
        waitpid(pID5, NULL, 0);

        pID7 = fork();
        if (pID7 == 0)
        {
            info(BEGIN, 7, 0); // inceput + final 7
            //------ crearea thread-urilor procesului 7------//
            sem_init(&semafor_thr7_max5, 0, 5);
            pthread_t threads_p7[48];
            int nr_threads_p7[48] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48};

            for (int i = 0; i < 48; i++)
            {
                pthread_create(&threads_p7[i], NULL, thread_fn_p7, &nr_threads_p7[i]);
            }
            for (int i = 0; i < 48; i++)
            {
                pthread_join(threads_p7[i], NULL);
            }
            info(END, 7, 0);
            _exit(0);
        }
        waitpid(pID7, NULL, 0);

        pID8 = fork();
        if (pID8 == 0)
        {
            info(BEGIN, 8, 0); // inceput + final 8
            info(END, 8, 0);
            _exit(0);
        }
        waitpid(pID8, NULL, 0);

        info(END, 2, 0); // final 2
        _exit(0);
    }
    waitpid(pID2, NULL, 0);

    info(END, 1, 0);
    return 0;
}
