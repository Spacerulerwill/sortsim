#include "shell_sort.h"

#define SHELL_SORT_SLEEP sleep_microseconds((uint64_t)(*args.speed * 50000.0f));

void shell_sort(struct SortFunctionArgs args) {
    size_t interval = args.count / 2;
    while (interval > 0) {
        for (size_t i = interval; i < args.count; i++) {
            SortValueType temp = args.values[i];
            args.sortStats->arrayAccesses++;
            size_t j = i;
            while (j >= interval && args.values[j - interval] > temp) {
                args.sortStats->comparisons++;
                args.values[j] = args.values[j - interval];
                args.sortStats->swaps++;
                args.sortStats->arrayWrites++;
                args.sortStats->arrayAccesses++;
                j -= interval;
                SHELL_SORT_SLEEP
            }
            args.values[j] = temp;
        }
        interval /= 2;
    } 
}