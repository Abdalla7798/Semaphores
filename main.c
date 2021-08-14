#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define TRUE 1
#define BUFFER_SIZE 10
#define N 5
#define T1 8

int data = 0;

int buf[BUFFER_SIZE];
int in = 0;
int out = 0;
sem_t full;
sem_t empty;
sem_t mutex;
sem_t lock1;
sem_t lock2;
int counter = 0;


void *mCounters(void *arg)
{
    int value;
    int *index = (int *)arg;
    int i;
    i = index[0];
while(TRUE)
{

    int rNum = rand() % 25;
    sleep(rNum);

    printf("Counter thread %d: received a message\n",i);

    sleep(3);

    sem_getvalue(&lock1, &value);

    if (value <= 0){

    printf("Counter thread %d: wating to write\n",i);

    }

    sem_wait(&lock1);

    data = data +1 ;
    printf("Counter thread %d: now adding to counter, counter value = %d\n",i,data);
    sleep(T1);

    sem_post(&lock1);
    sem_post(&lock2);
}
}


void *mmonitor_producer(void *arg)
{
    int item,produced;
    int value , value1;
    while(TRUE)
    {

    sem_getvalue(&lock1, &value);
    sem_getvalue(&lock2, &value1);

    if (value <= 0 || value1 <= 0){

    printf("Monitor thread: waiting to read counter\n");

    }

    //int rNum = rand() % 4;
    //sleep(rNum);

    sem_wait(&lock2);
    sem_wait(&lock1);

    produced = data;
    printf("Monitor thread: reading a count value of %d\n",data);
    data = 0;
    sleep(T1);

    sem_post(&lock1);


        item = produced; //////// produce
        if (counter == BUFFER_SIZE){
          printf("Monitor thread: Buffer full!\n");
        }

        sem_wait(&empty);  // check if buffer is full
        sem_wait(&mutex);

        buf[in] = item;
        printf("Monitor thread: writing to buffer at position %d\n",in);
        in = (in + 1) % N;
        counter++;
        sleep(T1);

        sem_post(&mutex);
        sem_post(&full);
    }
}


void *mcollector_consumer(void *arg)
{
    int consumed;

    while(TRUE)
    {

    //int rNum = rand() % 4;
    //sleep(rNum);

        if (counter == 0){
        printf("Collecter thread: nothing is in the buffer!\n");
        }

        sem_wait(&full); // check if buffer is empty
        sem_wait(&mutex);

        consumed = buf[out];
        printf("Collecter thread: reading from buffer at position %d\n",out);
        out = (out + 1) % N;
        counter--;
        sleep(T1);

        sem_post(&mutex);
        sem_post(&empty);
    }
}


int main()
{
   int i;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

   pthread_t *mCounter = (pthread_t*)malloc(N*sizeof(pthread_t));
   pthread_t pid, cid;
   sem_init(&full, 0, 0);
   sem_init(&empty, 0, BUFFER_SIZE);
   sem_init(&mutex, 0, 1);
   sem_init(&lock1, 0, 1);
   sem_init(&lock2, 0, 0);

   for (i = 0; i < N; i++) {

    int *index = (int *)malloc(2*sizeof(int));
    index[0] = i+1;
    pthread_create(&mCounter[i], &attr, mCounters, (void*)(index));
   }
    pthread_create(&pid, &attr, mmonitor_producer, NULL);
    pthread_create(&cid, &attr, mcollector_consumer, NULL);

   for (i = 0; i < N; i++){
    pthread_join(mCounter[i], NULL);
   }

   pthread_join(pid, NULL);
   pthread_join(cid, NULL);

    return 0;
}
