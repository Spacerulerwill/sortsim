#include "bogo_sort.h"

#define BOGO_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 1000.0f));

void bogo_sort(struct SortFunctionArgs args)
{
    struct SortStats *sortStats = args.sortStats;
    SortValueType *values = args.values;
    size_t count = args.count;
    while (!(is_already_sorted(values, count, sortStats)))
    {
        shuffle(values, count, sortStats);
        BOGO_SORT_SLEEP
        if (atomic_load(args.cancelSort))
            return;
    }
}