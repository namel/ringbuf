#ifndef __RINGBUF_H
#define __RINGBUF_H
#include <stdint.h>
//
//
// ringbuf
//
//
// a super fast producer-consumer ring buffer.
//
// assumptions:
//   - single producer, single consumer
//   - smp environment 
//   - properties negotiation
//   - consumer/producer never disconnect
//
//
// implementation:
//   - consumer must register first, listing the features it supports
//   - while the producer registers, the consumer is told 
//     which features where chosen
//   - actual data is not on the ring, just a pointer
//   - only producer can change a buffer entry from free to active
//     will busy-loop until it finds one free
//   - only consumer can change a buffer entry from active to free
//     will busy-loop until it finds one active
//   - at any given time, at most one of them will be spinning
//


#define RINGBUF__FEATURE_SPINLOCK              0x0001
#define RINGBUF__FEATURE_MUTEX_SLEEP_LOCK      0x0002
#define RINGBUF__FEATURE_ALL                   0xFFFF


// list of return codes
typedef enum {
        RINGBUF__OK,
        RINGBUF__NOMEM,
        RINGBUF__NOT_EMPTY,
        RINGBUF__NO_CONSUMER,
        RINGBUF__CONSUMER_EXISTS,
        RINGBUF__FEATURE_NOT_SUPPORTED,
        RINGBUF__PRODUCER_EXISTS,
        RINGBUF__BUFFER_FULL,
        RINGBUF__NO_RESOURCE,
} RINGBUF__errors_t;


// ring create and destroy
int ringbuf__create(uint64_t ring_size);
int ringbuf__destroy();

// consumer-provided function to enable the chosen features
typedef void (feature_enabling_func) (uint64_t chosen_features);

// registration of the consumer, the producer
int ringbuf__register_consumer(uint64_t features_supported, feature_enabling_func *f);
int ringbuf__register_producer(uint64_t *features_required);

// add or take one resource
int ringbuf__add_resource(void *resource, int blocking);
int ringbuf__take_resource(void **resource, int blocking);


// constants
#define CACHE_LINE_SIZE 128
#define TRUE 1
#define FALSE 0


#endif
