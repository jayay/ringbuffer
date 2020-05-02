#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include "ringbuffer.h"

static void ring_buffer_advance_head(ring_buffer* object, size_t bytes)
{
    assert(object);
    object->head = (object->head + bytes) % BUFFER_SIZE;
}

static void ring_buffer_advance_tail(ring_buffer* object, size_t bytes)
{
    assert(object);
    object->tail = (object->tail + bytes) % BUFFER_SIZE;
}

static inline void consumer_notify_producer(ring_buffer* object)
{
    assert(object);
    if (sem_post(&object->sem_w) < 0) {
        BAIL("sem_post failed");
    }
}

static inline void producer_wait_if_idle(ring_buffer* object)
{
    assert(object);
    if (sem_wait(&object->sem_w) < 0) {
        BAIL("sem_wait failed");
    }
}

static inline void producer_notify_consumer(ring_buffer* object)
{
    assert(object);
    if (sem_post(&object->sem_r) < 0) {
        BAIL("sem_post failed");
    }
}

static inline void consumer_wait_if_idle(ring_buffer* object)
{
    assert(object);
    if (sem_wait(&object->sem_r) < 0) {
        BAIL("sem_wait failed");
    }
}

int ring_buffer_init(ring_buffer** object)
{
    *object = malloc(sizeof(ring_buffer));
    if (*object == NULL) {
        return -1;
    }
    (*object)->head = ATOMIC_VAR_INIT(0);
    (*object)->tail = ATOMIC_VAR_INIT(0);
    (*object)->eof = ATOMIC_VAR_INIT(false);
    if (sem_init(&(*object)->sem_w, 0, BUFFER_SIZE - 1) < 0
        || sem_init(&(*object)->sem_r, 0, 0)) {
        BAIL("sem_init failed");
    }
    return 0;
}

size_t ring_buffer_write(ring_buffer* object, const char* data, const size_t len)
{
    size_t i;
    assert(object);
    for (i = 0; i < len; i++) {
        producer_wait_if_idle(object);
        object->buffer[object->head] = data[i];
        ring_buffer_advance_head(object, 1);
        producer_notify_consumer(object);
    }

    return len;
}

size_t ring_buffer_read(ring_buffer* object, char* data, const size_t max)
{
    size_t bytes_read;
    assert(object);
    for (bytes_read = 0; bytes_read < max; bytes_read += 1) {
        if (object->eof && object->head == object->tail) {
            break;
        }
        consumer_wait_if_idle(object);
        data[bytes_read] = object->buffer[object->tail];
        ring_buffer_advance_tail(object, 1);
        consumer_notify_producer(object);
    }
    return bytes_read;
}

void ring_buffer_set_eof(ring_buffer* object, bool eof)
{
    assert(object);
    object->eof = eof;
}

void ring_buffer_free(ring_buffer* object)
{
    if (object != NULL) {
        sem_destroy(&object->sem_w);
        sem_destroy(&object->sem_r);
        free(object);
        object = NULL;
    }
}