#ifndef SORTS_H
#define SORTS_H

#include <stdatomic.h>
#include <stdbool.h>

#include "visualizer.h"

/*
 * Sort functions take in:
 * A pointer to the sort stats function to update while sorting
 * A pointer to the array to sort
 * The length of the array to sort
 * An atomic boolean flag to periodically check to see if the sort should cancel
 * A uint64_t delay in microseconds
 */
typedef void (*SortFunction)(SortStats *, SortType *, SortType, _Atomic bool *, uint64_t);

typedef struct
{
    Visualizer *visualizer;
    SortFunction sort;
    _Atomic bool *isSorting;
    _Atomic bool *continueSorting;

} PerformSortParameter;

/*
 * This is the function passed to the thread to start the sort, it takes in a void
 * pointer because the <threads.h> interface must be generic. It should only be
 * passed a heap allocated PerformSortParameter struct cast to a void* which is freed
 * by the function itself, anything else will cause undefined behaviour
 */
int perform_sort(void *arg);
// Randomly shuffle elements in array
void shuffle(SortType *values, SortType count, SortStats *sortStats);
// Check if array is already sorted - useful in some sorts and to avoid unneccessary sorting
bool is_already_sorted(SortType *values, SortType count, SortStats *sortStats);

// Read only array containing function pointers to all sorts
extern const SortFunction sortFunctions[];
// How many sort functions we have
extern const size_t totalSorts;

#endif // !SORTS_H
