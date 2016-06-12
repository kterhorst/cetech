/*******************************************************************************
**** Generic dynamic array
*******************************************************************************/

#ifndef CETECH_ARRAY_H
#define CETECH_ARRAY_H

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "../memory/allocator.h"
#include "../errors/errors.h"
#include "../memory/memory.h"


/*******************************************************************************
**** Interface macros
*******************************************************************************/

#define ARRAY_T(T)                   struct array_##T

#define ARRAY_INIT(T, a, alloc)    array_init_##T(a, alloc)
#define ARRAY_DESTROY(T, a)        array_destroy_##T(a)

#define ARRAY_SIZE(a)              (a)->size
#define ARRAY_CAPACITY(a)          (a)->capacity
#define ARRAY_BEGIN(a)             (a)->data
#define ARRAY_END(a)               ((a)->data + (a)->size)
#define ARRAY_AT(a, i)             (a)->data[i]

#define ARRAY_RESIZE(T, a, s)       array_resize_##T(a, s)
#define ARRAY_RESERVE(T, a, s)      array_reserve_##T(a, s)

#define ARRAY_PUSH_BACK(T, a, i)   array_push_back_##T(a, i)
#define ARRAY_PUSH(T, a, i, c)     array_push_##T(a, i, c)
#define ARRAY_POP_BACK(T, a)       array_pop_back_##T(a)


/*******************************************************************************
**** Prototype macros
*******************************************************************************/

#define ARRAY_PROTOTYPE(T)                                                      \
    struct array_##T {                                                          \
        Alloc_t allocator;                                                      \
        T *data;                                                                \
        size_t size;                                                            \
        size_t capacity;                                                        \
    };                                                                          \
                                                                                \
    void array_init_##T(struct array_##T *array, Alloc_t allocator) {           \
        CE_ASSERT("array", array != NULL);                                      \
        CE_ASSERT("array", allocator != NULL);                                  \
        array->data = NULL;                                                     \
        array->size = 0;                                                        \
        array->capacity = 0;                                                    \
        array->allocator = allocator;                                           \
    }                                                                           \
                                                                                \
    void array_destroy_##T(struct array_##T *a) {                               \
        CE_ASSERT("array", a != NULL);                                          \
        CE_ASSERT("array", a->allocator != NULL);                               \
        alloc_free(a->allocator, a->data);                                      \
    }                                                                           \
                                                                                \
    void array_resize_##T(struct array_##T *a, size_t newsize);                 \
    void array_grow_##T(struct array_##T *a, size_t mincapacity);               \
                                                                                \
    void array_setcapacity_##T(struct array_##T *a, size_t  newcapacity) {      \
        CE_ASSERT("array", a != NULL);                                          \
        CE_ASSERT("array", a->allocator != NULL);                               \
                                                                                \
        if (newcapacity == a->capacity) {                                       \
            return;                                                             \
        }                                                                       \
                                                                                \
        if (newcapacity < a->size) {                                            \
            array_resize_##T(a, newcapacity);                                   \
        }                                                                       \
                                                                                \
        T* newdata = 0;                                                         \
        if (newcapacity > 0) {                                                  \
            newdata = (T*) alloc_alloc(a->allocator, sizeof(T) * newcapacity/*, __alignof(T)*/);\
            memory_copy(newdata, a->data, sizeof(T) * a->size);                 \
        }                                                                       \
                                                                                \
        alloc_free(a->allocator, a->data);                                      \
                                                                                \
        a->data = newdata;                                                      \
        a->capacity = newcapacity;                                              \
    }                                                                           \
                                                                                \
    void array_grow_##T(struct array_##T *a, size_t mincapacity) {              \
        CE_ASSERT("array", a != NULL);                                          \
        size_t newcapacity = a->capacity * 2 + 8;                               \
                                                                                \
        if (newcapacity < mincapacity) {                                        \
            newcapacity = mincapacity;                                          \
        }                                                                       \
                                                                                \
        array_setcapacity_##T(a, newcapacity);                                  \
    }                                                                           \
                                                                                \
    void array_resize_##T(struct array_##T *a, size_t newsize) {                \
        CE_ASSERT("array", a != NULL);                                          \
        if (newsize > a->capacity) {                                            \
            array_grow_##T(a, newsize);                                         \
        }                                                                       \
                                                                                \
        a->size = newsize;                                                      \
    }                                                                           \
                                                                                \
    void array_reserve_##T(struct array_##T *a, size_t new_capacity) {          \
        CE_ASSERT("array", a != NULL);                                          \
        if (new_capacity > a->capacity) {                                       \
            array_setcapacity_##T(a, new_capacity);                             \
        }                                                                       \
    }                                                                           \
                                                                                \
    void array_push_back_##T(struct array_##T *a, T item) {                     \
        CE_ASSERT("array", a != NULL);                                          \
        if (a->size + 1 > a->capacity) {                                        \
            array_grow_##T(a, 0);                                               \
        }                                                                       \
        a->data[a->size++] = item;                                              \
    }                                                                           \
                                                                                \
    void array_push_##T(struct array_##T *a, T* items, size_t count) {          \
        CE_ASSERT("array", a != NULL);                                          \
        if (a->capacity <= a->size + count) {                                   \
            array_grow_##T(a, a->size + count);                                 \
        }                                                                       \
                                                                                \
        memory_copy(&a->data[a->size], items, sizeof(T) * count);               \
        a->size += count;                                                       \
    }                                                                           \
                                                                                \
    void array_pop_back_##T(struct array_##T *a) {                              \
        CE_ASSERT("array", a != NULL);                                          \
        CE_ASSERT("array", a->size != 0);                                       \
                                                                                \
        --a->size;                                                              \
    }                                                                           \


/*******************************************************************************
**** Predefined arary for prim types
*******************************************************************************/

ARRAY_PROTOTYPE(char)
ARRAY_PROTOTYPE(short)
ARRAY_PROTOTYPE(int)
ARRAY_PROTOTYPE(long)

#endif //CETECH_ARRAY_H
