#include "merge_sort.h"

#define MERGE_SORT_SLEEP sleep_microseconds((uint64_t)(*args->speed * 10000.0f));

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
    }

static void merge(size_t low, size_t mid, size_t high, struct SortFunctionArgs *args)
{
    struct SortStats *sortStats = args->sortStats;
    SortValueType *values = args->values;                                                                                                                 \
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

static void merge_sort_impl(size_t low, size_t high, struct SortFunctionArgs *args)
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

void merge_sort(struct SortFunctionArgs args)
{
    if (args.count < 2)
        return;
    merge_sort_impl(0, args.count - 1, &args);
}