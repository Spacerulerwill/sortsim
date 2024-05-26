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

int perform_sort(void *arg)
{
    Visualizer *visualizer = (Visualizer *)arg;
    sort_stats_reset(&visualizer->sortStats);
    SortFunctionArgs sortFunctionArgs = {&visualizer->sortStats, visualizer->values, visualizer->count,
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

void shuffle(SortValueType *values, size_t count, SortStats *sortStats)
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

bool is_already_sorted(SortValueType *values, size_t count, SortStats *sortStats)
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

static void swap(SortStats *stats, SortValueType *a, SortValueType *b)
{
    SortValueType temp = *a;
    *a = *b;
    *b = temp;
    stats->swaps++;
    stats->arrayAccesses += 2;
    stats->arrayWrites += 2;
}

static void bubble_sort(SortFunctionArgs args)
{
#define BUBBLE_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));
    bool swapped = true;
    SortStats *sortStats = args.sortStats;
    SortValueType *values = args.values;
    size_t count = args.count;
    for (size_t i = 0; i < count - 1 && swapped; i++)
    {
        swapped = false;
        for (size_t j = 0; j < count - i - 1; j++)
        {
            if (values[j] > values[j + 1])
            {
                swap(sortStats, &values[j + 1], &values[j]);
                swapped = true;
            }
            sortStats->comparisons++;
            BUBBLE_SORT_SLEEP
            if (atomic_load(args.cancelSort))
                return;
        }
    }
#undef BUBBLE_SORT_SLEEP
}

static void selection_sort(SortFunctionArgs args)
{
#define SELECTION_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));
    SortStats *sortStats = args.sortStats;
    SortValueType *values = args.values;
    size_t count = args.count;
    size_t min_idx = 0;
    for (size_t i = 0; i < count - 1; i++)
    {
        min_idx = i;
        for (size_t j = i + 1; j < count; j++)
        {
            if (values[j] < values[min_idx])
                min_idx = j;
            sortStats->comparisons++;
            SELECTION_SORT_SLEEP
            if (atomic_load(args.cancelSort))
                return;
        }
        swap(sortStats, &values[min_idx], &values[i]);
    }
#undef SELECTION_SORT_SLEEP
}

static void insertion_sort(SortFunctionArgs args)
{
#define INSERTION_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));
    SortStats *sortStats = args.sortStats;
    SortValueType *values = args.values;
    size_t count = args.count;
    SortValueType key = 0;
    for (size_t i = 1; i < count; i++)
    {
        key = values[i];
        sortStats->arrayAccesses++;
        size_t j = i;
        while (j > 0 && values[j - 1] > key)
        {
            sortStats->comparisons++;
            values[j] = values[j - 1];
            sortStats->arrayWrites++;
            sortStats->swaps++;
            j--;
            INSERTION_SORT_SLEEP
            if (atomic_load(args.cancelSort))
                return;
        }
        values[j] = key;
        sortStats->arrayWrites++;
    }
#undef INSERTION_SORT_SLEEP
}

static void cocktail_shaker_sort(SortFunctionArgs args)
{
#define COCKTAIL_SHAKER_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));
    SortStats *sortStats = args.sortStats;
    SortValueType *values = args.values;
    size_t count = args.count;
    size_t left = 0;
    size_t right = count - 1;
    bool swapped = true;
    while (left < right && swapped)
    {
        swapped = false;
        // Move the largest element to the end
        for (size_t i = left; i < right; ++i)
        {
            if (values[i] > values[i + 1])
            {
                swap(sortStats, &values[i], &values[i + 1]);
                COCKTAIL_SHAKER_SORT_SLEEP
                if (atomic_load(args.cancelSort))
                    return;
                swapped = true;
            }
            sortStats->comparisons++;
        }
        right--;

        // Move the smallest element to the beginning
        for (size_t i = right; i > left; --i)
        {
            if (values[i] < values[i - 1])
            {
                swap(sortStats, &values[i], &values[i - 1]);
                COCKTAIL_SHAKER_SORT_SLEEP
                if (atomic_load(args.cancelSort))
                    return;
                swapped = true;
            }
            sortStats->comparisons++;
        }
        left++;
    }
#undef COCKTAIL_SHAKER_SORT_SLEEP
}

#define QUICK_SORT_SLEEP sleep_microseconds((uint64_t)(*args->speed * 50000.0f));
static size_t quicksort_partition(size_t low, size_t high, SortFunctionArgs *args)
{
    SortStats *sortStats = args->sortStats;
    SortValueType *values = args->values;

    SortValueType pivot = values[low];
    size_t i = low + 1;
    size_t j = high;
    sortStats->comparisons++;
    while (i <= j)
    {
        sortStats->comparisons++;
        while (i <= high && values[i] <= pivot)
        {
            i++;
        }
        sortStats->comparisons++;
        while (j >= low && values[j] > pivot)
        {
            j--;
        }
        if (i < j)
        {
            swap(sortStats, &values[i], &values[j]);
            QUICK_SORT_SLEEP
            if (atomic_load(args->cancelSort))
                return 0;
        }
    }
    swap(sortStats, &values[low], &values[j]);
    QUICK_SORT_SLEEP
    if (atomic_load(args->cancelSort))
        return 0;
    return j;
}

static void quicksort_impl(size_t low, size_t high, SortFunctionArgs *args)
{
    if (low >= high)
        return;

    size_t partition_idx = quicksort_partition(low, high, args);
    if (atomic_load(args->cancelSort))
        return;

    if (partition_idx > 0)
        quicksort_impl(low, partition_idx - 1, args);
    quicksort_impl(partition_idx + 1, high, args);
}

static void quick_sort(SortFunctionArgs args)
{
    quicksort_impl(0, args.count - 1, &args);
}
#undef QUICK_SORT_SLEEP

#define MERGE_SORT_SLEEP sleep_microseconds((uint64_t)(*args->speed * 10000.0f));
static void merge(size_t low, size_t mid, size_t high, SortFunctionArgs *args)
{
    SortStats *sortStats = args->sortStats;
    SortValueType *values = args->values;
    /*
     * This macro is helpful as the merge function uses heap allocated memory. Instead of repeating this large
     * block of code in every place where we need to exit the sort we use this macro.
     */
#define CONTINUE_SORT_CHECK                                                                                            \
    if (atomic_load(args->cancelSort))                                                                                 \
    {                                                                                                                  \
        free(leftSide);                                                                                                \
        free(rightSide);                                                                                               \
        return;                                                                                                        \
    }                                                                                                                  \
    // Create temp arrays and copy data to them
    size_t leftSize = mid - low + 1;
    size_t rightSize = high - mid;
    SortValueType *leftSide = malloc(leftSize * sizeof(SortValueType));
    if (leftSide == NULL)
    {
        fputs("Failed to allocate memory to left side during merge sort\n", stderr);
        exit(EXIT_FAILURE);
    }
    SortValueType *rightSide = malloc(rightSize * sizeof(SortValueType));
    if (rightSide == NULL)
    {
        fputs("Failed to allocate memory to right side during merge sort\n", stderr);
        free(leftSide);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < leftSize; i++)
    {
        sortStats->arrayWrites++;
        sortStats->arrayAccesses++;
        leftSide[i] = values[low + i];
        CONTINUE_SORT_CHECK
    }
    for (size_t j = 0; j < rightSize; j++)
    {
        sortStats->arrayWrites++;
        sortStats->arrayAccesses++;
        rightSide[j] = values[mid + 1 + j];
        CONTINUE_SORT_CHECK
    }
    // Merge temp arrays back
    size_t i = 0;
    size_t j = 0;
    size_t k = low;
    while (i < leftSize && j < rightSize)
    {
        sortStats->comparisons++;
        if (leftSide[i] <= rightSide[j])
        {
            values[k] = leftSide[i];
            sortStats->arrayWrites++;
            MERGE_SORT_SLEEP
            CONTINUE_SORT_CHECK
            i++;
        }
        else
        {
            values[k] = rightSide[j];
            sortStats->arrayWrites++;
            MERGE_SORT_SLEEP
            CONTINUE_SORT_CHECK
            j++;
        }
        k++;
    }

    // Copy the remaining elements
    while (i < leftSize)
    {
        values[k] = leftSide[i];
        sortStats->arrayWrites++;
        MERGE_SORT_SLEEP
        CONTINUE_SORT_CHECK
        i++;
        k++;
    }
    while (j < rightSize)
    {
        values[k] = rightSide[j];
        sortStats->arrayWrites++;
        MERGE_SORT_SLEEP
        CONTINUE_SORT_CHECK
        j++;
        k++;
    }
    free(leftSide);
    free(rightSide);
#undef CONTINUE_SORT_CHECK
}

static void merge_sort_impl(size_t low, size_t high, SortFunctionArgs *args)
{
    if (low >= high)
        return;
    if (atomic_load(args->cancelSort))
        return;
    size_t mid = low + (high - low) / 2;
    merge_sort_impl(low, mid, args);
    merge_sort_impl(mid + 1, high, args);
    merge(low, mid, high, args);
}

static void merge_sort(SortFunctionArgs args)
{
    if (args.count < 2)
        return;
    merge_sort_impl(0, args.count - 1, &args);
}
#undef MERGE_SORT_SLEEP

static void bogo_sort(SortFunctionArgs args)
{
#define BOGO_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));
    SortStats *sortStats = args.sortStats;
    SortValueType *values = args.values;
    size_t count = args.count;
    while (!(is_already_sorted(values, count, sortStats)))
    {
        shuffle(values, count, sortStats);
        BOGO_SORT_SLEEP
        if (atomic_load(args.cancelSort))
            return;
    }
#undef BOGO_SORT_SLEEP
}

const SortFunction sortFunctions[] = {bubble_sort, selection_sort, insertion_sort, cocktail_shaker_sort,
                                      quick_sort,  merge_sort,     bogo_sort};
const size_t totalSorts = sizeof(sortFunctions) / sizeof(SortFunction);
