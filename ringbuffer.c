#include <stdlib.h>
#include "ringbuffer.h"

static void ring_buffer_advance_head(ring_buffer* object)
{
    object->head = (object->head + 1) % BUFFER_SIZE;
}

static void ring_buffer_advance_tail(ring_buffer* object)
{
    object->tail = (object->tail + 1) % BUFFER_SIZE;
}

static inline void consumer_notify_producer(ring_buffer* object)
{
    sem_post(&object->sem_w);
}

static inline void producer_wait_if_idle(ring_buffer* object)
{
    sem_wait(&object->sem_w);
}

static inline void producer_notify_consumer(ring_buffer* object)
{
    sem_post(&object->sem_r);
}

static inline void consumer_wait_if_idle(ring_buffer* object)
{
    sem_wait(&object->sem_r);
}

int ring_buffer_init(ring_buffer** object)
{
    *object = malloc(sizeof(ring_buffer));
    if (*object == NULL) {
        return -1;
    }
    (*object)->head = 0;
    (*object)->tail = 0;
    sem_init(&(*object)->sem_w, 0, BUFFER_SIZE);
    sem_init(&(*object)->sem_r, 0, 0);
    (*object)->eof = false;
    return 0;
}

size_t ring_buffer_write(ring_buffer* object, char* data, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++) {
        producer_wait_if_idle(object);
        object->buffer[object->head] = data[i];
        ring_buffer_advance_head(object);
        producer_notify_consumer(object);
    }

    return len;
}

size_t ring_buffer_read(ring_buffer* object, char* data, size_t max)
{
    size_t bytes_read;
    for (bytes_read = 0; bytes_read < max; bytes_read += 1) {
        if (object->eof && object->head == object->tail) {
            break;
        }
        consumer_wait_if_idle(object);
        data[bytes_read] = object->buffer[object->tail];
        ring_buffer_advance_tail(object);
        consumer_notify_producer(object);
    }
    return bytes_read;
}

int ring_buffer_set_eof(ring_buffer* object, bool eof)
{
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