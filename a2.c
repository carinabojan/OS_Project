#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

sem_t sem;
sem_t sem1;
sem_t *sem2 = NULL;
sem_t *sem3 = NULL;
sem_t sem4;
sem_t sem5;

int end11;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

int runningThreads;

void* threadFn(void *arg)
{
    int i = *((int*)arg);

    if(i == 1)
    {
        sem_wait(&sem);
        info(BEGIN, 9, 1);
        info(END, 9, 1);
        sem_post(&sem1);
    }
    else if(i == 3)
    {
        info(BEGIN, 9, 3);
        sem_post(&sem);
        sem_wait(&sem1);
        info(END, 9, 3);
    }
    else if(i == 4)
    {
        sem_wait(sem2);
        info(BEGIN, 9, 4);
        info(END, 9, 4);
        sem_post(sem3);
    }
    else
    {

        info(BEGIN, 9, i);

        info(END, 9, i);
    }

    return NULL;
}


void* threadFn1(void *arg)
{
    int i = *((int*)arg);

    if(i == 2)
    {
        sem_wait(sem3);
        info(BEGIN, 3, 2);
        info(END, 3, 2);
    }
    else if(i == 4)
    {
        info(BEGIN, 3, 4);
        info(END, 3, 4);
        sem_post(sem2);
    }
    else
    {

        info(BEGIN, 3, i);

        info(END, 3, i);
    }

    return NULL;
}

void* threadFn2(void *arg)
{
    int i = *((int*)arg);
    sem_wait(&sem4);

    if(i < 5)
    {
        info(BEGIN, 8, i);
        pthread_mutex_lock(&lock);
          runningThreads++;
          if(runningThreads == 4){
            pthread_cond_signal(&cond1);
          }
          while(end11 == 0){
            pthread_cond_wait(&cond, &lock);
          }
        pthread_mutex_unlock(&lock);

        info(END, 8, i);
    }
    else if(i == 11)
    {
        info(BEGIN, 8, 11);
        pthread_mutex_lock(&lock1);
        while(runningThreads != 4){
            pthread_cond_wait(&cond1, &lock1);
        }
        pthread_mutex_unlock(&lock1);
        info(END, 8, 11);
        end11 = 1;
        pthread_cond_broadcast(&cond);
    }
    else
    {
        info(BEGIN, 8, i);

        info(END, 8, i);
    }

    sem_post(&sem4);
    return NULL;
}

int main()
{

    pid_t pid[9];
    init();

    info(BEGIN, 1, 0);

    pid[1] = fork();/// aici am P1 si P2
    if(pid[1] == -1)
    {
        perror("Could not create child process");
        return -1;
    }

    if(pid[1] == 0)  ///P2
    {
        info(BEGIN, 2, 0);
        pid[2] = fork(); /// P1, P2, P3
        if(pid[2] == - 1)
        {
            perror("Could not create child process");
            return -1;
        }
        if(pid[2] == 0)  /// P3
        {
            info(BEGIN, 3, 0);
            pid[5] = fork(); ///P1, P2, P3, P6
            if(pid[5] == - 1)
            {
                perror("Could not create child process");
                return -1;
            }
            if(pid[5] == 0) /// P6
            {
                info(BEGIN, 6, 0);
                pid[6] = fork(); /// P1, P2, P3, P6, P7
                if(pid[6] == - 1)
                {
                    perror("Could not create child process");
                    return -1;
                }
                if(pid[6] == 0) /// P7
                {
                    info(BEGIN, 7, 0);
                    info(END, 7, 0);
                }
                else
                {
                    wait(&pid[6]);
                    info(END, 6, 0);
                }
            }
            else
            {

                pthread_t tid1[6];
                int ind1[6];

                sem2 = sem_open("T3.4_T9.4", O_CREAT, 0644, 0);
                sem3 = sem_open("T9.4_T3.2", O_CREAT, 0644, 0);

                for (int i = 1; i <= 6; i++)
                {

                    ind1[i-1] = i;
                    pthread_create(&tid1[i-1], NULL, threadFn1, &ind1[i-1]);
                }

                for (int i = 1; i <= 6; i++)
                {
                    pthread_join(tid1[i-1], NULL);
                }

                wait(&pid[5]);
                info(END, 3, 0);
            }
        }
        else
        {
            wait(&pid[2]);
            info(END, 2, 0);
        }
    }
    else
    {
        /// P1
        pid[3] = fork();/// P1, P2, P3, P6, P7, P4
        if(pid[3] == - 1)
        {
            perror("Could not create child process");
            return -1;
        }
        if(pid[3] == 0) /// P4
        {
            info(BEGIN, 4, 0);
            info(END, 4, 0);
        }
        else
        {
            pid[4] = fork(); /// P1, P2, P3, P6, P7, P4, P5
            if(pid[4] == - 1)
            {
                perror("Could not create child process");
                return -1;
            }
            if(pid[4] == 0) /// P5
            {
                info(BEGIN, 5, 0);
                info(END, 5, 0);
            }
            else
            {
                pid[7] = fork(); /// P1, P2, P3, P6, P7, P4, P5, P8
                if(pid[7] == - 1)
                {
                    perror("Could not create child process");
                    return -1;
                }
                if(pid[7] == 0) /// P8
                {
                    info(BEGIN, 8, 0);
                    pid[8] = fork(); /// P1, P2, P3, P6, P7, P4, P5, P8, P9
                    if(pid[8] == - 1) ///P9
                    {
                        perror("Could not create child process");
                        return -1;
                    }
                    if(pid[8] == 0) /// P9
                    {
                        info(BEGIN, 9, 0);

                        sem2 = sem_open("T3.4_T9.4", O_CREAT, 0644, 0);
                        sem3 = sem_open("T9.4_T3.2", O_CREAT, 0644, 0);

                        pthread_t tid[5];
                        int ind[5];

                        for (int i = 1; i <= 5; i++)
                        {

                            ind[i-1] = i;

                            pthread_create(&tid[i-1], NULL, threadFn, &ind[i-1]);
                        }

                        for (int i = 1; i <= 5; i++)
                        {
                            pthread_join(tid[i-1], NULL);
                        }

                        info(END, 9, 0);
                    }
                    else
                    {

                        pthread_t tid2[42];
                        int ind2[42];
                        int i1 = 1,i2 = 2,i3 = 3,i4 = 4, i11 = 11;

                        sem_init(&sem4, 0, 5);
                        sem_init(&sem5, 0, 0);

                        pthread_create(&tid2[0], NULL, threadFn2, &i1);
                        pthread_create(&tid2[1], NULL, threadFn2, &i2);
                        pthread_create(&tid2[2], NULL, threadFn2, &i3);
                        pthread_create(&tid2[3], NULL, threadFn2, &i4);
                        pthread_create(&tid2[10], NULL, threadFn2, &i11);

                        for (int i = 5; i <= 42; i++)
                        {
                            if(i != 11)
                            {
                                ind2[i-1] = i;
                                pthread_create(&tid2[i-1], NULL, threadFn2, &ind2[i-1]);
                            }
                        }

                        for (int i = 1; i <= 42; i++)
                        {
                            pthread_join(tid2[i-1], NULL);
                        }

                        sem_destroy(&sem4);
                        sem_destroy(&sem5);

                        wait(&pid[8]);
                        info(END, 8, 0);
                    }
                }
                else
                {
                    wait(&pid[1]);
                    wait(&pid[3]);
                    wait(&pid[4]);
                    wait(&pid[7]);
                    info(END, 1, 0);
                }
            }
        }
    }
    return 0;
}//sfarsit
