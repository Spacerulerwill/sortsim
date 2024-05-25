#ifndef SORTS_H
#define SORTS_H

#include <stdatomic.h>
#include <stdbool.h>

#include "visualizer.h"

typedef void (*SortFunction)(SortStats *, SortType *, SortType, _Atomic bool *);

typedef struct
{
    Visualizer *visualizer;
    SortFunction sort;
    _Atomic bool *isSorting;
    _Atomic bool *continueSorting;

} PerformSortParameter;

int perform_sort(void *arg);
void shuffle(SortType *values, SortType count, SortStats *sortStats);
bool is_already_sorted(SortType *values, SortType count, SortStats *sortStats);

typedef struct
{
    SortType *values;
    SortType *size;
} SortParameter;

extern const SortFunction sortFunctions[];
extern const size_t totalSorts;

#endif // !SORTS_H
