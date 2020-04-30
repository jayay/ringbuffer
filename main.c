#include <stdio.h>
#include <pthread.h>
#include "ringbuffer.h"

static void* consumer_fn(void* buffer) {
    char buf[20];
    ring_buffer *rb = (ring_buffer*) buffer;
    int len;

    for (int j = 0; j < 600; j++) {
        len = ring_buffer_read(rb, buf, 19);
        for (int i = 0; i < len; i+=1) {
            printf("%hhu ", buf[i]);
        }
    }
    return NULL;
}

static void* producer_fn(void* buffer) {
    char counter = 0;
    ring_buffer *rb = (ring_buffer*) buffer;
    for (int i = 0; i < 600; i++) {
        counter = i % 255;
        if (ring_buffer_write(rb, &counter, 1) < 0) {
            puts("Writing error\n");
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
        perror("allocating ring buffer failed");
    }

    if (pthread_create(&producer, NULL, producer_fn, rb) != 0) {
        perror("Producer thread failed");
    }

    if (pthread_create(&consumer, NULL, consumer_fn, rb) != 0) {
        perror("Consumer thread failed");
    }

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    ring_buffer_free(rb);
}