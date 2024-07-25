#include "sorts.h"
#include "bubble_sort.h"
#include "selection_sort.h"
#include "insertion_sort.h"
#include "shell_sort.h"
#include "cocktail_shaker_sort.h"
#include "quick_sort.h"
#include "merge_sort.h"
#include "heap_sort.h"
#include "bogo_sort.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <threads.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix__)
#include <errno.h>
#include <time.h>
#endif

void sleep_microseconds(uint64_t microseconds)
{
#if defined(_WIN32)
    // Ugly busy waiting for microsecond precision sleep
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);

    long long elapsedMicroseconds = 0;
    while (elapsedMicroseconds < (long long)microseconds)
    {
        QueryPerformanceCounter(&end);
        elapsedMicroseconds = (end.QuadPart - start.QuadPart) * 1000000LL / frequency.QuadPart;
    }
#elif defined(__unix__)
    struct timespec ts = {0, (long int)microseconds * 1000L};
    nanosleep(&ts, NULL);
#endif
}

int perform_sort(void *arg)
{
    struct Visualizer *visualizer = (struct Visualizer *)arg;
    sort_stats_reset(&visualizer->sortStats);
    struct SortFunctionArgs sortFunctionArgs = {&visualizer->sortStats, visualizer->values, visualizer->count,
                                         &visualizer->cancelSort, &visualizer->speed};
    sortFunctions[visualizer->selectedSort](sortFunctionArgs);
    if (atomic_load(&visualizer->cancelSort))
    {
        for (size_t i = 0; i < visualizer->count; i++)
        {
            visualizer->values[i] = (SortValueType)(visualizer->count - i);
        }
        shuffle(visualizer->values, visualizer->count, NULL);
        sort_stats_reset(&visualizer->sortStats);
        atomic_store(&visualizer->cancelSort, false);
    }
    atomic_store(&visualizer->isSorting, false);
    return 0;
}

void shuffle(SortValueType *values, size_t count, struct SortStats *sortStats)
{
    if (count > 1)
    {
        size_t i;
        for (i = 0; i < count - 1; i++)
        {
            size_t j = i + (size_t)rand() / (RAND_MAX / (count - i) + 1);
            SortValueType t = values[j];
            values[j] = values[i];
            values[i] = t;
            if (sortStats)
            {
                sortStats->swaps++;
                sortStats->arrayAccesses += 2;
                sortStats->arrayWrites += 2;
            }
        }
    }
}

bool is_already_sorted(SortValueType *values, size_t count, struct SortStats *sortStats)
{
    for (size_t i = 0; i < count - 1; i++)
    {
        if (sortStats)
        {
            sortStats->arrayAccesses += 2;
            sortStats->comparisons += 1;
        }
        if (values[i] > values[i + 1])
        {
            return false;
        }
    }
    return true;
}

void swap(struct SortStats *stats, SortValueType *a, SortValueType *b)
{
    SortValueType temp = *a;
    *a = *b;
    *b = temp;
    stats->swaps++;
    stats->arrayAccesses += 2;
    stats->arrayWrites += 2;
}

const SortFunction sortFunctions[] = {bubble_sort, selection_sort, insertion_sort, shell_sort, cocktail_shaker_sort,
                                      quick_sort,  merge_sort,     heap_sort, bogo_sort};
const size_t totalSorts = sizeof(sortFunctions) / sizeof(SortFunction);
