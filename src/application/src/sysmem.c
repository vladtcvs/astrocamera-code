#include <stddef.h>
#include <stdint.h>
#include <errno.h>

extern uint8_t _end;               /* End of BSS, start of heap */
extern uint8_t _estack;            /* Top of stack (end of RAM) */
extern uint32_t _Min_Stack_Size;   /* Minimum stack size reserved */

static uint8_t *heap_end = NULL;

void *_sbrk(ptrdiff_t incr)
{
    if (heap_end == NULL) {
        heap_end = &_end;  // Initialize heap_end on first call
    }

    uint8_t *prev_heap_end = heap_end;
    uint8_t *stack_limit = (uint8_t *)(&_estack) - (uint32_t)&_Min_Stack_Size;
    uint8_t *new_heap_end = heap_end + incr;

    // Check for heap/stack collision
    if (new_heap_end > stack_limit) {
        // Heap and stack collision -> fail
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end = new_heap_end;
    return (void *)prev_heap_end;
}
