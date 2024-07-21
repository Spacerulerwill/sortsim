#include "cocktail_shaker_sort.h"

void cocktail_shaker_sort(SortFunctionArgs args)
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