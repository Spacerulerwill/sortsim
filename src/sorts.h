#ifndef SORTS_H
#define SORTS_H

#include "visualizer.h"
#include <stdatomic.h>
#include <stdbool.h>

// Arguments required for a sort function grouped here for easier passing around
typedef struct
{
    SortStats *sortStats;
    SortValueType *values;
    size_t count;
    _Atomic bool *cancelSort;
    float *speed;
} SortFunctionArgs;

// The function pointer of a sort function
typedef void (*SortFunction)(SortFunctionArgs);

int perform_sort(void *arg);
// When we pass the perform_sort to a thread it can only take one argument, a void*, so
// we pass this and cast it in the function! C is so safe...
typedef struct
{
    Visualizer *visualizer;
    SortFunction sort;
} PerformSortArgs;

void shuffle(SortValueType *values, size_t count, SortStats *sortStats);
bool is_already_sorted(SortValueType *values, size_t count, SortStats *sortStats);

extern const SortFunction sortFunctions[];
extern const size_t totalSorts;

#endif // !SORTS_H
