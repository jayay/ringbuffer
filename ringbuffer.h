#include <stdatomic.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define BUFFER_SIZE 64

#define BAIL(message) \
    fprintf(stderr, "Error: %s\n", (message));\
    exit(EXIT_FAILURE);

typedef struct ring_buffer {
     _Atomic size_t head;
     _Atomic size_t tail;
    sem_t sem_w;
    sem_t sem_r;
    _Atomic bool eof;
    char buffer[BUFFER_SIZE];
} ring_buffer;


int ring_buffer_init(ring_buffer** object);

size_t ring_buffer_write(ring_buffer* object, const char* data, const size_t len);

void ring_buffer_set_eof(ring_buffer* object, bool eof);

size_t ring_buffer_read(ring_buffer* object, char* data, const size_t max);

void ring_buffer_free(ring_buffer* object);