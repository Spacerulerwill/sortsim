#ifndef SORTS_H
#define SORTS_H

#include "visualizer.h"
#include <stdatomic.h>
#include <stdbool.h>

// Arguments required for a sort function grouped here for easier passing around
struct SortFunctionArgs {
    struct SortStats *sortStats;
    SortValueType *values;
    size_t count;
    _Atomic bool *cancelSort;
    float *speed;
};

// The function pointer of a sort function
typedef void (*SortFunction)(struct SortFunctionArgs);

int perform_sort(void *arg);
// When we pass the perform_sort to a thread it can only take one argument, a void*, so
// we pass this and cast it in the function! C is so safe...
struct PerformSortArgs {
    struct Visualizer *visualizer;
    SortFunction sort;
};

// Shuffle the sorting array
void shuffle(SortValueType *values, size_t count, struct SortStats *sortStats);
// Determine if all elements are in ascending order
bool is_already_sorted(SortValueType *values, size_t count, struct SortStats *sortStats);
// Swap two elements in the sorting array
void swap(struct SortStats *stats, SortValueType *a, SortValueType *b);
// sleep current thread for a specified amount of microseconds
void sleep_microseconds(uint64_t microseconds);

// Sort function forward declarations
void bubble_sort(struct SortFunctionArgs args);
void selection_sort(struct SortFunctionArgs args);
void insertion_sort(struct SortFunctionArgs args);
void shell_sort(struct SortFunctionArgs args);
void cocktail_shaker_sort(struct SortFunctionArgs args);
void quick_sort(struct SortFunctionArgs args);
void merge_sort(struct SortFunctionArgs args);
void heap_sort(struct SortFunctionArgs args);
void bogo_sort(struct SortFunctionArgs args);

// All sorts
extern const SortFunction sortFunctions[];
extern const size_t totalSorts;

#endif // !SORTS_H
