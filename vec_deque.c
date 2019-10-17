#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define VEC_INITIAL_CAPACITY 2

/*
    A growable ring buffer
    inspired by VecDeque from the Rust standard library.
    https://doc.rust-lang.org/std/collections/struct.VecDeque.html
    This implements the 'queue' of search cells.
    I originally just used a basic growable array, but this was an interesting
    experiment and should end up using slightly less memory since the memory
    for older cells gets reused for newer ones (hence 'ring buffer').
*/

typedef int vec_item_t;

typedef struct {
    vec_item_t *storage;
    /* where data should be written */
    size_t head;
    /* first data which can be read */
    size_t tail;
    size_t capacity;
} vec_deque_t;

void vec_init(vec_deque_t *vec);
void vec_push_back(vec_deque_t *vec, vec_item_t item);
bool vec_pop_front(vec_deque_t *vec, vec_item_t *item);
void vec_free(vec_deque_t *vec);

void vec_init(vec_deque_t *vec) {
    vec->storage =
        (vec_item_t *)malloc(sizeof(vec_item_t) * VEC_INITIAL_CAPACITY);
    assert(vec->storage != NULL);
    vec->head = 0;
    vec->tail = 0;
    vec->capacity = VEC_INITIAL_CAPACITY;
}

void vec_push_back(vec_deque_t *vec, vec_item_t item) {
    printf("inserting at %ld\n", vec->head);
    vec->storage[vec->head] = item;
    vec->head = (vec->head + 1) % vec->capacity;

    /* buffer is full */
    if(vec->head == vec->tail) {
        printf("buffer is now full\n");
        /* now need to reallocate */
        size_t old_cap = vec->capacity;
        vec->capacity *= 2;
        vec->storage = (vec_item_t *)realloc(vec->storage, sizeof(vec_item_t) *
                                                                vec->capacity);
        /* if the head was at the end of the buffer, no need to move memory */
        if(vec->head == 0) {
            vec->head = old_cap;
            /* noop */
        /* if the head was smaller than the tail, move it */
        } else if(vec->head < old_cap - vec->tail) {
            printf("head smaller than tail! copying %ld bytes\n", vec->head);
            memcpy(vec->storage + old_cap, vec->storage, sizeof(vec_item_t) * vec->head);
            vec->head += old_cap;
        /* if the tail was smaller than the head, move it right to the end */
        } else {
            printf("tail smaller than head!\n");
            size_t tail_size = old_cap - vec->tail;
            size_t next_tail = vec->capacity - tail_size;
            memcpy(vec->storage + next_tail, vec->storage + vec->tail, sizeof(vec_item_t) * tail_size);
            vec->tail = next_tail;
        }
    }

    assert(vec->head < vec->capacity);
    assert(vec->tail < vec->capacity);

}

bool vec_pop_front(vec_deque_t *vec, vec_item_t *item) {
    /* empty queue */
    if(vec->head == vec->tail) {
        return false;
    }

    *item = vec->storage[vec->tail];
    vec->storage[vec->tail] = 0;
    vec->tail = (vec->tail + 1) % vec->capacity;
    return true;
}

void vec_free(vec_deque_t *vec) {
    free(vec->storage);
    vec->storage = NULL;
}

void vec_print(vec_deque_t *vec) {
    printf("|");
    for(size_t i = 0; i < vec->capacity; i++) {
        printf("#%1ld|", i);
    }
    printf("\n");
    printf("|");
    for(size_t i = 0; i < vec->capacity; i++) {
        printf("%2d|", vec->storage[i]);
    }
    printf("\n");
    printf("- Capacity: %ld\n", vec->capacity);
    printf("- Head: %ld\n", vec->head);
    printf("- Tail: %ld\n", vec->tail);
}

int main() {
    vec_deque_t vec;
    vec_init(&vec);
    vec_item_t item;
    while(true) {
        if(scanf("%d", &item) == 1) {
            vec_push_back(&vec, item);
            printf("Printing vec:\n");
            vec_print(&vec);
        } else if(scanf("pop %d", &item)) {
            vec_pop_front(&vec, &item);
            printf("Popped: %d\n", item);
            printf("Printing vec:\n");
            vec_print(&vec);
        }
    }
    vec_free(&vec);
}