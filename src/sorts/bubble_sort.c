#include "bubble_sort.h"

#define BUBBLE_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));

void bubble_sort(SortFunctionArgs args)
{
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
}