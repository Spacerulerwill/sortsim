#include "quick_sort.h"

#define QUICK_SORT_SLEEP sleep_microseconds((uint64_t)(*args->speed * 50000.0f));

static size_t quicksort_partition(size_t low, size_t high, struct SortFunctionArgs *args)
{
    struct SortStats *sortStats = args->sortStats;
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

static void quicksort_impl(size_t low, size_t high, struct SortFunctionArgs *args)
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

void quick_sort(struct SortFunctionArgs args)
{
    quicksort_impl(0, args.count - 1, &args);
}