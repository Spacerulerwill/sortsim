#include "selection_sort.h"

#define SELECTION_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));

void selection_sort(SortFunctionArgs args)
{
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
}