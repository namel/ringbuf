#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ringbuf.h"


#define RING_SIZE 1024
#define NUM_ITER 100000


int features_enabled = FALSE;
void feature_enabler(uint64_t features)
{
        features_enabled = TRUE;
}

void *consumer(void *arg)
{

        uint32_t resource=1;
        int rc;

        // register the consumer
        rc = ringbuf__register_consumer(RINGBUF__FEATURE_ALL, &feature_enabler);
        while (!features_enabled);

        // consume resources
        while (resource) {
                rc = ringbuf__take_resource((void **)&resource, TRUE);
                if (rc != RINGBUF__OK) {
                        printf("bad rc from add-resource (%d)\n", rc);
                        exit(-1);
                }

                printf("resource = %d\n", resource);
        }

        pthread_exit(NULL);
}


void *producer(void *arg)
{
        static uint64_t iter = NUM_ITER;
        static uint32_t fibo1 = 0;
        static uint32_t fibo2 = 0;
        static uint32_t fibo3 = 1;
        uint64_t features = (uint32_t) arg;
        int rc = ~RINGBUF__OK;


        // register the producer
        while(rc != RINGBUF__OK)
                rc = ringbuf__register_producer(&features);

        // add a predetermined number of resources to the ring
        while(iter--) {

                // add the next fibonacci number
                // last iteration sends a zero
                rc = ringbuf__add_resource((void *)(iter==1?0:fibo3), TRUE);
                if (rc != RINGBUF__OK) {
                        printf("bad rc from add-resource (%d)\n", rc);
                        exit(-1);
                }


                // calc the next fibonacci number
                fibo1 = fibo2;
                fibo2 = fibo3;
                fibo3 = fibo1 + fibo2;
        }

        // done
        pthread_exit(NULL);
}


int main(int argc, char **argv)
{
        pthread_t consumer_thread;
        pthread_t producer_thread;
        int rc;

        // create the ring
        rc = ringbuf__create(RING_SIZE);
        if (rc != RINGBUF__OK) { printf("bad rc = %d\n", rc); exit(-1); }

        // start the consumer
        rc = pthread_create(&consumer_thread, NULL, consumer, NULL);
        if (rc != RINGBUF__OK) { printf("bad rc = %d\n", rc); exit(-1); }

        // start the producer
        uint32_t features = RINGBUF__FEATURE_SPINLOCK;
        if (argc == 2 && (0 == strcmp(argv[1], "sleeplock"))) features = RINGBUF__FEATURE_MUTEX_SLEEP_LOCK;
        rc = pthread_create(&producer_thread, NULL, producer, (void *) features);
        if (rc != RINGBUF__OK) { printf("bad rc = %d\n", rc); exit(-1); }

        // destroy the ring
        rc = ringbuf__destroy();

        pthread_exit(NULL);
}

