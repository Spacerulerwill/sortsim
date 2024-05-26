#include "sorts.h"

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

static void sleep_microseconds(uint64_t microseconds)
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
    Visualizer *visualizer = param->visualizer;
    atomic_store(param->isSorting, true);
    atomic_store(param->continueSorting, true);
    sort_stats_reset(&visualizer->sortStats);
    param->sort(&visualizer->sortStats, visualizer->values, visualizer->count, param->continueSorting,
                visualizer->delay);
    if (atomic_load(param->continueSorting))
    {
        atomic_store(param->continueSorting, false);
    }
    else
    {
        for (SortType i = 0; i < visualizer->count; i++)
        {
            visualizer->values[i] = visualizer->count - i;
        }
        shuffle(visualizer->values, visualizer->count, NULL);
        sort_stats_reset(&visualizer->sortStats);
    }
    atomic_store(param->isSorting, false);
    free(param);
    return 0;
}

void bubble_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting, uint64_t delay)
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
            sleep_microseconds(delay);
            if (!atomic_load(continueSorting))
                return;
        }
    }
}

void selection_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting, uint64_t delay)
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
            sleep_microseconds(delay);
            if (!atomic_load(continueSorting))
                return;
        }
        swap(stats, &values[min_idx], &values[i]);
    }
}

void insertion_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting, uint64_t delay)
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
            sleep_microseconds(delay);
            if (!atomic_load(continueSorting))
                return;
        }
        values[j] = key;
        stats->array_writes++;
    }
}

void cocktail_shaker_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting,
                          uint64_t delay)
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
                sleep_microseconds(delay);
                if (!atomic_load(continueSorting))
                    return;
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
                sleep_microseconds(delay);
                if (!atomic_load(continueSorting))
                    return;
                swapped = true;
            }
            stats->comparisons++;
        }
        left++;
    }
}

void bogo_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting, uint64_t delay)
{
    while (!(is_already_sorted(values, count, stats)))
    {
        shuffle(values, count, stats);
        sleep_microseconds(delay);
        if (!atomic_load(continueSorting))
            return;
    }
}

static SortType quicksort_partition(SortType low, SortType high, SortStats *stats, SortType *values,
                                    _Atomic bool *continueSorting, uint64_t delay)
{
    SortType pivot = values[low];
    SortType i = low + 1;
    SortType j = high;
    stats->comparisons++;
    while (i <= j)
    {
        stats->comparisons++;
        while (i <= high && values[i] <= pivot)
        {
            i++;
        }
        stats->comparisons++;
        while (j >= low && values[j] > pivot)
        {
            j--;
        }
        if (i < j)
        {
            swap(stats, &values[i], &values[j]);
            sleep_microseconds(delay);
            if (!atomic_load(continueSorting))
                return 0;
        }
    }
    swap(stats, &values[low], &values[j]);
    sleep_microseconds(delay);
    if (!atomic_load(continueSorting))
        return 0;
    return j;
}

static void quicksort_impl(SortType low, SortType high, SortStats *stats, SortType *values,
                           _Atomic bool *continueSorting, uint64_t delay)
{
    if (low >= high)
        return;

    SortType partition_idx = quicksort_partition(low, high, stats, values, continueSorting, delay);
    if (!atomic_load(continueSorting))
        return;

    if (partition_idx > 0)
        quicksort_impl(low, partition_idx - 1, stats, values, continueSorting, delay);
    quicksort_impl(partition_idx + 1, high, stats, values, continueSorting, delay);
}

void quicksort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting, uint64_t delay)
{
    quicksort_impl(0, count - 1, stats, values, continueSorting, delay);
}

static void merge(SortStats *stats, SortType *values, SortType low, SortType mid, SortType high,
                  _Atomic bool *continueSorting, uint64_t delay)
{
    /*
     * This macro is helpful as the merge function uses heap allocated memory. Instead of repeating this large
     * block of code in every place where we need to exit the sort we use this macro.
     */
#define CONTINUE_SORT_CHECK                                                                                            \
    if (!atomic_load(continueSorting))                                                                                 \
    {                                                                                                                  \
        free(leftSide);                                                                                                \
        free(rightSide);                                                                                               \
        return;                                                                                                        \
    }                                                                                                                  \
    // Create temp arrays and copy data to them
    SortType leftSize = mid - low + 1;
    SortType rightSize = high - mid;
    SortType *leftSide = malloc(leftSize * sizeof(SortType));
    if (leftSide == NULL)
    {
        fputs("Failed to allocate memory to left side during merge sort\n", stderr);
        exit(EXIT_FAILURE);
    }
    SortType *rightSide = malloc(rightSize * sizeof(SortType));
    if (rightSide == NULL)
    {
        fputs("Failed to allocate memory to right side during merge sort\n", stderr);
        free(leftSide);
        exit(EXIT_FAILURE);
    }
    for (SortType i = 0; i < leftSize; i++)
    {
        stats->array_writes++;
        stats->array_accesses++;
        leftSide[i] = values[low + i];
        CONTINUE_SORT_CHECK
    }
    for (SortType j = 0; j < rightSize; j++)
    {
        stats->array_writes++;
        stats->array_accesses++;
        rightSide[j] = values[mid + 1 + j];
        CONTINUE_SORT_CHECK
    }
    // Merge temp arrays back
    SortType i = 0;
    SortType j = 0;
    SortType k = low;
    while (i < leftSize && j < rightSize)
    {
        stats->comparisons++;
        if (leftSide[i] <= rightSide[j])
        {
            values[k] = leftSide[i];
            stats->array_writes++;
            sleep_microseconds(delay);
            CONTINUE_SORT_CHECK
            i++;
        }
        else
        {
            values[k] = rightSide[j];
            stats->array_writes++;
            sleep_microseconds(delay);
            CONTINUE_SORT_CHECK
            j++;
        }
        k++;
    }

    // Copy the remaining elements
    while (i < leftSize)
    {
        values[k] = leftSide[i];
        stats->array_writes++;
        sleep_microseconds(delay);
        CONTINUE_SORT_CHECK
        i++;
        k++;
    }
    while (j < rightSize)
    {
        values[k] = rightSide[j];
        stats->array_writes++;
        sleep_microseconds(delay);
        CONTINUE_SORT_CHECK
        j++;
        k++;
    }
    free(leftSide);
    free(rightSide);
#undef CONTINUE_SORT_CHECK
}

static void merge_sort_impl(SortStats *stats, SortType *values, SortType low, SortType high,
                            _Atomic bool *continueSorting, uint64_t delay)
{
    if (low >= high)
        return;
    if (!atomic_load(continueSorting))
        return;
    SortType mid = low + (high - low) / 2;
    merge_sort_impl(stats, values, low, mid, continueSorting, delay);
    merge_sort_impl(stats, values, mid + 1, high, continueSorting, delay);
    merge(stats, values, low, mid, high, continueSorting, delay);
}

void merge_sort(SortStats *stats, SortType *values, SortType count, _Atomic bool *continueSorting, uint64_t delay)
{
    if (count < 2)
        return;
    merge_sort_impl(stats, values, 0, count - 1, continueSorting, delay);
}

const SortFunction sortFunctions[] = {bubble_sort, selection_sort, insertion_sort, cocktail_shaker_sort,
                                      bogo_sort,   quicksort,      merge_sort};
const size_t totalSorts = sizeof(sortFunctions) / sizeof(SortFunction);
