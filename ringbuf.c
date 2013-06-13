#include <stdlib.h>
#include "ringbuf.h"
#include "ringbuf_internal.h"

int ringbuf__create(uint64_t ring_size)
{
        int i;
        resource_t *ring = malloc(ring_size * sizeof(resource_t));
        if (!ring)
                return -RINGBUF__NOMEM;

        rbg.ring = ring;
        rbg.ring_size = ring_size;
        for (i=0; i<ring_size; i++) {
                pthread_mutex_init(&ring[i].mutex, NULL);
                pthread_cond_init(&ring[i].cond, NULL);
        }

        return RINGBUF__OK;
}

int ringbuf__destroy()
{
        // TODO: not implemented 
        return RINGBUF__OK;
}


int ringbuf__register_consumer(uint64_t features_supported, feature_enabling_func *f)
{
        if (rbg.consumer_registered)
                return -RINGBUF__CONSUMER_EXISTS;

        rbg.add = spinlock_add;
        rbg.take = spinlock_take;
        rbg.consumer_registered = TRUE;
        rbg.features_supported = features_supported;
        rbg.feature_enabler = f;

        return RINGBUF__OK;
}

int ringbuf__register_producer(uint64_t *features_required)
{
        if (!rbg.consumer_registered)
                return -RINGBUF__NO_CONSUMER;

        if (rbg.producer_registered)
                return -RINGBUF__PRODUCER_EXISTS;

        uint64_t missing_features = *features_required & ~rbg.features_supported;

        if (missing_features) {
                *features_required = rbg.features_supported;
                return -RINGBUF__FEATURE_NOT_SUPPORTED;
        }

        rbg.producer_registered = TRUE;
        rbg.features_enabled = *features_required;
        if (rbg.features_enabled & RINGBUF__FEATURE_MUTEX_SLEEP_LOCK) {
                rbg.add = condvar_add;
                rbg.take = condvar_take;
        }

        if (rbg.feature_enabler)
                (*rbg.feature_enabler)(rbg.features_enabled);

        return RINGBUF__OK;
}


inline uint64_t get_next_index(uint64_t index)
{
        return (index + 1) % rbg.ring_size;
}


int ringbuf__add_resource(void *resource, int blocking)
{
        uint64_t next = get_next_index(rbg.last_resource_added);
        resource_t *r = &rbg.ring[next];

        if (!blocking && r->used) 
                return -RINGBUF__BUFFER_FULL;

        (*rbg.add)(r, resource);
        rbg.last_resource_added = next;

        return RINGBUF__OK;
}


int ringbuf__take_resource(void **resource, int blocking)
{
        uint64_t next = get_next_index(rbg.last_resource_taken);
        resource_t *r = &rbg.ring[next];

        if (!blocking && !r->used)
                return -RINGBUF__NO_RESOURCE;

        (*rbg.take)(r, resource);
        rbg.last_resource_taken = next;

        return RINGBUF__OK;
}



