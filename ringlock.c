#include <assert.h>
#include "ringbuf_internal.h"

void spinlock_add(resource_t *r, void *resource)
{
        // spin
        while (r->used);

        // add resource
        r->resource = resource;
        __sync_synchronize();
        r->used = TRUE;
}


void spinlock_take(resource_t *r, void **resource)
{
        // spin
        while (!r->used);

        // take resource
        *resource = r->resource;
        __sync_synchronize();
        r->used = FALSE;
}


void condvar_add(resource_t *r, void *resource)
{
        // take mutex with unused slot
        while(1) {
                pthread_mutex_lock(&r->mutex);
                if (!r->used)
                        break;
                pthread_mutex_unlock(&r->mutex);
        }

        // add resource, signal and release
        r->resource = resource;
        r->used = TRUE;
        pthread_cond_signal(&r->cond);
        pthread_mutex_unlock(&r->mutex);
}


void condvar_take(resource_t *r, void **resource)
{
        // take the mutex or wake with mutex
        pthread_mutex_lock(&r->mutex);
        if (r->used == FALSE)
                pthread_cond_wait(&r->cond, &r->mutex);

        // take resource and release mutex
        assert(r->used);
        *resource = r->resource;
        r->used = FALSE;
        pthread_mutex_unlock(&r->mutex);
}



