#include <stdio.h>
#include <pthread.h>
#include "ringbuffer.h"

static void* consumer_fn(void* buffer) {
    char buf[20];
    ring_buffer *rb = (ring_buffer*) buffer;
    int len;

    do {
        len = ring_buffer_read(rb, buf, 19);
        for (int i = 0; i < len; i+=1) {
            printf("%hhu ", buf[i]);
        }
    } while (len > 0);
    return NULL;
}

static void* producer_fn(void* buffer) {
    char counter = 0;
    ring_buffer *rb = (ring_buffer*) buffer;
    for (int i = 0; i < 600; i++) {
        counter = i % 255;
        if (ring_buffer_write(rb, &counter, 1) < 0) {
            return NULL;
        }
    }
    ring_buffer_set_eof(rb, true);
    return NULL;
}

int main(int argc, char** argv) {
    pthread_t producer, consumer;

    ring_buffer* rb;
    if (ring_buffer_init(&rb) < 0) {
        BAIL("allocating ring buffer failed");
    }

    if (pthread_create(&producer, NULL, producer_fn, rb) != 0) {
        BAIL("Producer thread failed");
    }

    if (pthread_create(&consumer, NULL, consumer_fn, rb) != 0) {
        BAIL("Consumer thread failed");
    }

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    ring_buffer_free(rb);
}