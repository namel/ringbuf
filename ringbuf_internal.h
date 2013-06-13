#include <pthread.h>
#include "ringbuf.h"

#pragma pack(push, CACHE_LINE_SIZE)
typedef struct {
        int used;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        void *resource;
} resource_t;

typedef struct {
        uint64_t ring_size;
        int consumer_registered;
        int producer_registered;
        feature_enabling_func *feature_enabler;
        uint64_t features_supported;
        uint64_t features_enabled;
        uint64_t last_resource_added;
        uint64_t last_resource_taken;
        void (*add)(resource_t *r, void *resource);
        void (*take)(resource_t *r, void **resource);
        resource_t *ring;
} ringbuf_globals_t;


ringbuf_globals_t rbg; 
#pragma pack(pop)

// ring add/take implementations
void spinlock_add(resource_t *r, void *resource);
void spinlock_take(resource_t *r, void **resource);
void condvar_add(resource_t *r, void *resource);
void condvar_take(resource_t *r, void **resource);


