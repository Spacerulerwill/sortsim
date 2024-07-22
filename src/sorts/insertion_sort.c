#include "insertion_sort.h"

#define INSERTION_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));

void insertion_sort(struct SortFunctionArgs args)
{
    struct SortStats *sortStats = args.sortStats;
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
}