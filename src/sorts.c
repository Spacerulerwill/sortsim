#include "sorts.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <threads.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined (__unix__)
#include <time.h>
#include <errno.h>
#endif

#define DELAY_MICROSECONDS 50

static void sleep_microseconds(uint64_t microseconds)
{
    #if defined(_WIN32)
    // Ugly busy waiting for microsecond precision sleep
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);

    long long elapsedMicroseconds = 0;
    while (elapsedMicroseconds < microseconds)
    {
        QueryPerformanceCounter(&end);
        elapsedMicroseconds = (end.QuadPart - start.QuadPart) * 1000000LL / frequency.QuadPart;
    }
    #elif defined(__unix__)
    struct timespec ts = {0, (long int)microseconds * 1000L};
    nanosleep(&ts, NULL);
    #endif
}

bool is_already_sorted(SortType *values, SortType count, SortStats *sortStats)
{
    for (SortType i = 0; i < count - 1; i++)
    {
        if (sortStats)
        {
            sortStats->array_accesses += 2;
            sortStats->comparisons += 1;
        }
        if (values[i] > values[i + 1])
        {
            return false;
        }
    }
    return true;
}

void shuffle(SortType *values, SortType count, SortStats *stats)
{
    size_t n = (size_t)count;
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
            size_t j = i + (size_t)rand() / (RAND_MAX / (n - i) + 1);
            SortType t = values[j];
            values[j] = values[i];
            values[i] = t;
            if (stats)
            {
                stats->swaps++;
                stats->array_accesses += 2;
                stats->array_writes += 2;
            }
        }
    }
}

static void swap(SortStats *stats, SortType *a, SortType *b)
{
    SortType temp = *a;
    *a = *b;
    *b = temp;
    stats->swaps++;
    stats->array_accesses += 2;
    stats->array_writes += 2;
}

int perform_sort(void *arg)
{
    PerformSortParameter *param = (PerformSortParameter *)arg;
    atomic_store(param->isSorting, true);
    atomic_store(param->continueSorting, true);
    sort_stats_reset(&param->visualizer->sortStats);
    param->sort(&param->visualizer->sortStats, param->visualizer->values, param->visualizer->count,
                param->continueSorting);
    if (atomic_load(param->continueSorting))
    {
        atomic_store(param->continueSorting, false);
    }
    else
    {
        for (SortType i = 0; i < param->visualizer->count; i++)
        {
            param->visualizer->values[i] = param->visualizer->count - i;
        }
        shuffle(param->visualizer->values, param->visualizer->count, NULL);
        sort_stats_reset(&param->visualizer->sortStats);
    }
    atomic_store(param->isSorting, false);
    free(param);
    return 0;
}

void bubble_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting)
{
    SortType i, j;
    bool swapped = true;
    for (i = 0; i < count - 1 && swapped; i++)
    {
        swapped = false;
        for (j = 0; j < count - i - 1; j++)
        {
            if (values[j] > values[j + 1])
            {
                swap(stats, &values[j + 1], &values[j]);
                swapped = true;
            }
            stats->comparisons++;
            sleep_microseconds(DELAY_MICROSECONDS);
            if (!atomic_load(continueSorting))
            {
                return;
            }
        }
    }
}

void selection_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting)
{
    SortType i, j, min_idx;
    for (i = 0; i < count - 1; i++)
    {
        min_idx = i;
        for (j = i + 1; j < count; j++)
        {
            if (values[j] < values[min_idx])
                min_idx = j;
            stats->comparisons++;
            sleep_microseconds(DELAY_MICROSECONDS);
            if (!atomic_load(continueSorting))
            {
                return;
            }
        }
        swap(stats, &values[min_idx], &values[i]);
    }
}

void insertion_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting)
{
    SortType i, key, j;
    for (i = 1; i < count; i++)
    {
        key = values[i];
        stats->array_accesses++;
        j = i;
        while (j > 0 && values[j - 1] > key)
        {
            stats->comparisons++;
            values[j] = values[j - 1];
            stats->array_writes++;
            stats->swaps++;
            j--;
            sleep_microseconds(DELAY_MICROSECONDS);
            if (!atomic_load(continueSorting))
            {
                return;
            }
        }
        values[j] = key;
        stats->array_writes++;
    }
}

void cocktail_shaker_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting)
{
    SortType left = 0;
    SortType right = count - 1;
    bool swapped = true;

    while (left < right && swapped)
    {
        swapped = false;

        // Move the largest element to the end
        for (int i = left; i < right; ++i)
        {
            if (values[i] > values[i + 1])
            {
                swap(stats, &values[i], &values[i + 1]);
                sleep_microseconds(DELAY_MICROSECONDS);
                if (!atomic_load(continueSorting))
                {
                    return;
                }
                swapped = true;
            }
            stats->comparisons++;
        }
        right--;

        // Move the smallest element to the beginning
        for (int i = right; i > left; --i)
        {
            if (values[i] < values[i - 1])
            {
                swap(stats, &values[i], &values[i - 1]);
                sleep_microseconds(DELAY_MICROSECONDS);
                if (!atomic_load(continueSorting))
                {
                    return;
                }
                swapped = true;
            }
            stats->comparisons++;
        }
        left++;
    }
}

void bogo_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting)
{
    while (!(is_already_sorted(values, count, stats)))
    {
        shuffle(values, count, stats);
        sleep_microseconds(DELAY_MICROSECONDS);
        if (!atomic_load(continueSorting))
        {
            return;
        }
    }
}

const SortFunction sortFunctions[] = {bubble_sort, selection_sort, insertion_sort, cocktail_shaker_sort, bogo_sort};
const size_t totalSorts = sizeof(sortFunctions) / sizeof(SortFunction);
